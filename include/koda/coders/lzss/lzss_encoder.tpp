#pragma once

#include <koda/utils/utils.hpp>

namespace koda {

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr LzssEncoder<Token, AuxiliaryEncoder, Allocator>::LzssEncoder(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryEncoder auxiliary_encoder,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    : dictionary_and_buffer_{FusedDictAndBufferInfo{
          dictionary_size, std::move(cyclic_buffer_size)}},
      search_tree_{look_ahead_size, allocator},
      auxiliary_encoder_{std::move(auxiliary_encoder)} {}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr LzssEncoder<Token, AuxiliaryEncoder, Allocator>::LzssEncoder(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    requires std::is_default_constructible_v<AuxiliaryEncoder>
    : LzssEncoder{dictionary_size, look_ahead_size, AuxiliaryEncoder{},
                  std::move(cyclic_buffer_size), allocator} {}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
[[nodiscard]] constexpr auto&&
LzssEncoder<Token, AuxiliaryEncoder, Allocator>::auxiliary_encoder(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.auxiliary_encoder_);
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssEncoder<Token, AuxiliaryEncoder, Allocator>::Flush(
    BitOutputRange auto&& output) {
    return auxiliary_encoder_.Flush(
        FlushData(std::forward<decltype(output)>(output)));
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssEncoder<Token, AuxiliaryEncoder, Allocator>::Encode(
    InputRange<Token> auto&& input, BitOutputRange auto&& output) {
    if (std::holds_alternative<FusedDictAndBufferInfo>(
            dictionary_and_buffer_)) {
        return EncodeData(InitializeBuffer(input), output);
    }
    return EncodeData(input, output);
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto
LzssEncoder<Token, AuxiliaryEncoder, Allocator>::InitializeBuffer(
    InputRange<Token> auto&& input) {
    const size_t look_ahead_size = search_tree_.string_size();

    std::vector<Token> init_view{std::from_range,
                                 input | std::views::take(look_ahead_size)};

    auto [dict_size, cyclic_buffer_size] =
        std::get<FusedDictAndBufferInfo>(dictionary_and_buffer_);
    dictionary_and_buffer_ = FusedDictionaryAndBuffer{
        dict_size, SequenceView{init_view}, std::move(cyclic_buffer_size),
        search_tree_.get_allocator()};

    return input | std::views::drop(look_ahead_size);
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssEncoder<Token, AuxiliaryEncoder, Allocator>::EncodeData(
    InputRange<Token> auto&& input, BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<Token>>(
        dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<Token>>(dictionary_and_buffer_);

    auto out_range = AsSubrange(std::forward<decltype(output)>(output));
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);
    for (; (input_iter != input_sent) && !out_range.empty(); ++input_iter) {
        auto look_ahead = dict.get_buffer();
        out_range = PeformEncodigStep(dict, look_ahead, std::move(out_range));
        search_tree_.AddString(look_ahead);
        dict.AddSymbolToBuffer(*input_iter);
    }
    return CoderResult{std::move(input_iter), std::move(input_sent),
                       std::move(out_range)};
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto
LzssEncoder<Token, AuxiliaryEncoder, Allocator>::EncodeTokenOrMatch(
    Token token, const Match& match, BitOutputRange auto&& output) {
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

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto
LzssEncoder<Token, AuxiliaryEncoder, Allocator>::PeformEncodigStep(
    FusedDictionaryAndBuffer<Token>& dict, SequenceView look_ahead,
    BitOutputRange auto&& output) {
    if (!match_count_) {
        auto [_, new_output] = EncodeTokenOrMatch(
            look_ahead[0], search_tree_.FindMatch(look_ahead),
            std::move(output));
        TryToRemoveStringFromSearchTree(dict);
        return new_output;
    }

    --match_count_;
    TryToRemoveStringFromSearchTree(dict);
    return output;
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr void LzssEncoder<Token, AuxiliaryEncoder, Allocator>::
    TryToRemoveStringFromSearchTree(FusedDictionaryAndBuffer<Token>& dict) {
    if (dict.dictionary_size() == dict.max_dictionary_size()) {
        search_tree_.RemoveString(dict.get_oldest_dictionary_full_match());
    }
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssEncoder<Token, AuxiliaryEncoder, Allocator>::FlushData(
    BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<Token>>(
        dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<Token>>(dictionary_and_buffer_);
    auto out_range = AsSubrange(std::forward<decltype(output)>(output));

    for (size_t i = 0; i < search_tree_.string_size(); ++i) {
        out_range =
            PeformEncodigStep(dict, dict.get_buffer(), std::move(out_range));
        dict.AddEndSymbolToBuffer();
    }
    return out_range;
}

}  // namespace koda
