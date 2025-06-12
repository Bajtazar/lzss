#pragma once

namespace koda {

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::LzssEncoder(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryEncoder auxiliary_encoder,
    std::optional<size_t> cyclic_buffer_size, const AllocatorTp& allocator)
    : dictionary_and_buffer_{FusedDictAndBufferInfo{
          dictionary_size, std::move(cyclic_buffer_size)}},
      search_tree_{look_ahead_size, allocator},
      auxiliary_encoder_{std::move(auxiliary_encoder)} {}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::LzssEncoder(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size, const AllocatorTp& allocator)
    requires std::is_default_constructible_v<AuxiliaryEncoder>
    : LzssEncoder{dictionary_size, look_ahead_size, AuxiliaryEncoder{},
                  std::move(cyclic_buffer_size), allocator} {}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
[[nodiscard]] constexpr auto&&
LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::auxiliary_encoder(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.auxiliary_encoder_);
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr auto LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::Flush(
    BitOutputRange auto&& output) {
    return auxiliary_encoder_.Flush(
        FlushData(std::forward<decltype(output)>(output)));
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr auto LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::Encode(
    InputRange<InputToken> auto&& input, BitOutputRange auto&& output) {
    if (std::holds_alternative<FusedDictAndBufferInfo>(
            dictionary_and_buffer_)) {
        return EncodeData(InitializeBuffer(input), output);
    }
    return EncodeData(input, output);
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr auto
LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::InitializeBuffer(
    InputRange<InputToken> auto&& input) {
    const size_t look_ahead_size = search_tree_.string_size();

    std::vector<InputToken> init_view{
        std::from_range, input | std::views::take(look_ahead_size)};

    auto [dict_size, cyclic_buffer_size] =
        std::get<FusedDictAndBufferInfo>(dictionary_and_buffer_);
    dictionary_and_buffer_ = FusedDictionaryAndBuffer{
        dict_size, SequenceView{init_view}, std::move(cyclic_buffer_size),
        search_tree_.get_allocator()};

    return input | std::views::drop(look_ahead_size);
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr auto
LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::EncodeData(
    InputRange<InputToken> auto&& input, BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<InputToken>>(
        dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<InputToken>>(dictionary_and_buffer_);

    std::ranges::subrange out_range{std::ranges::begin(output),
                                    std::ranges::end(output)};

    for (auto token : input) {
        auto look_ahead = dict.get_buffer();
        out_range = PeformEncodigStep(dict, look_ahead, std::move(out_range));
        search_tree_.AddString(look_ahead);
        dict.AddSymbolToBuffer(token);
    }
    return out_range;
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr auto
LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::EncodeTokenOrMatch(
    InputToken token, const Match& match, BitOutputRange auto&& output) {
    IMToken symbol_token{token};

    if (!match) {
        return auxiliary_encoder_.Encode(
            std::ranges::subrange{&symbol_token, std::next(&symbol_token)},
            output);
    }

    IMToken match_token{static_cast<uint32_t>(match.match_position),
                        static_cast<uint16_t>(match.match_length)};

    float est_match_bitsize = auxiliary_encoder_.TokenBitSize(match_token);
    float est_symbol_bitsize = auxiliary_encoder_.TokenBitSize(symbol_token);

    if (est_symbol_bitsize <= est_match_bitsize) {
        return auxiliary_encoder_.Encode(
            std::ranges::subrange{&symbol_token, std::next(&symbol_token)},
            output);
    }
    match_count_ = match.match_length - 1;
    return auxiliary_encoder_.Encode(
        std::ranges::subrange{&match_token, std::next(&match_token)}, output);
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr auto
LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::PeformEncodigStep(
    FusedDictionaryAndBuffer<InputToken>& dict, SequenceView look_ahead,
    BitOutputRange auto&& output) {
    if (!match_count_) {
        auto new_output = EncodeTokenOrMatch(look_ahead[0],
                                             search_tree_.FindMatch(look_ahead),
                                             std::move(output));
        TryToRemoveStringFromSearchTree(dict);
        return new_output;
    }

    --match_count_;
    TryToRemoveStringFromSearchTree(dict);
    return output;
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr void LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::
    TryToRemoveStringFromSearchTree(
        FusedDictionaryAndBuffer<InputToken>& dict) {
    if (dict.dictionary_size() == dict.max_dictionary_size()) {
        search_tree_.RemoveString(dict.get_oldest_dictionary_full_match());
    }
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr auto
LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::FlushData(
    BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<InputToken>>(
        dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<InputToken>>(dictionary_and_buffer_);

    std::ranges::subrange out_range{std::ranges::begin(output),
                                    std::ranges::end(output)};

    for (size_t i = 0; i < search_tree_.string_size(); ++i) {
        out_range =
            PeformEncodigStep(dict, dict.get_buffer(), std::move(out_range));
        dict.AddEndSymbolToBuffer();
    }
    return out_range;
}

}  // namespace koda
