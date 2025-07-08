#pragma once

#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/utils.hpp>

namespace koda {

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Lz77Encoder(
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
constexpr Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Lz77Encoder(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    requires std::is_default_constructible_v<AuxiliaryEncoder>
    : Lz77Encoder{dictionary_size, look_ahead_size, AuxiliaryEncoder{},
                  std::move(cyclic_buffer_size), allocator} {}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
[[nodiscard]] constexpr auto&&
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::auxiliary_encoder(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.auxiliary_encoder_);
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Flush(
    BitOutputRange auto&& output) {
    return auxiliary_encoder_.Flush(
        FlushData(std::forward<decltype(output)>(output)));
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Encode(
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
constexpr auto
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::InitializeBuffer(
    InputRange<Token> auto&& input) {
    // buffer also stores one suffix symbol that is not used during match lookup
    // but is used to construct an intermediate token!
    const size_t buffer_size = 1 + search_tree_.string_size();

    std::vector<Token> init_view{std::from_range,
                                 input | std::views::take(buffer_size)};

    auto [dict_size, cyclic_buffer_size] =
        std::get<FusedDictAndBufferInfo>(dictionary_and_buffer_);
    dictionary_and_buffer_ = FusedDictionaryAndBuffer{
        dict_size, SequenceView{init_view}, std::move(cyclic_buffer_size),
        search_tree_.get_allocator()};

    return input | std::views::drop(buffer_size);
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::FlushQueue(
    BitOutputRange auto&& output) {
    auto [input_range, output_range] = auxiliary_encoder_.Encode(
        std::ranges::subrange{&(*queued_token_), std::next(&(*queued_token_))},
        output);
    if (std::ranges::begin(input_range) == std::ranges::end(input_range)) {
        queued_token_ = std::nullopt;
    }
    return output_range;
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::EncodeData(
    InputRange<Token> auto&& input, BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<Token>>(
        dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<Token>>(dictionary_and_buffer_);

    auto out_range = AsSubrange(std::forward<decltype(output)>(output));

    if (queued_token_) {
        out_range = FlushQueue(out_range);
        if (queued_token_) {
            return CoderResult{std::forward<decltype(input)>(input),
                               std::move(out_range)};
        }
    }

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
constexpr auto
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::EncodeTokenOrMatch(
    SequenceView look_ahead, const Match& match, BitOutputRange auto&& output) {
    // buffer is one symbol longer than the actual look-ahead buffer
    auto suffix_token = look_ahead[match.match_length];
    IMToken symbol_token{suffix_token, match.match_position,
                         match.match_length};

    match_count_ = match.match_length;
    return EncodeIntermediateToken(std::move(symbol_token),
                                   std::forward<decltype(output)>(output));
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::PeformEncodigStep(
    FusedDictionaryAndBuffer<Token>& dict, SequenceView look_ahead,
    BitOutputRange auto&& output) {
    if (!match_count_) {
        auto new_output = EncodeTokenOrMatch(
            look_ahead,
            // Prune the maximum match length suffix symbol from the look ahead
            search_tree_.FindMatch(
                StringView{look_ahead.begin(), std::prev(look_ahead.end())}),
            std::move(output));
        TryToRemoveStringFromSearchTree(dict);
        return new_output;
    }

    --match_count_;
    TryToRemoveStringFromSearchTree(dict);
    return AsSubrange(output);
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::EncodeIntermediateToken(
    IMToken&& token, BitOutputRange auto&& output) {
    auto [input_range, output_range] = auxiliary_encoder_.Encode(
        std::ranges::subrange{&token, std::next(&token)}, output);
    // Token has not been encoded due to the jam in the encoder queue - enque
    // token and try to flush it when new output range is provided by the user
    if (std::ranges::begin(input_range) != std::ranges::end(input_range)) {
        queued_token_ = token;
    }
    return output_range;
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr void Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::
    TryToRemoveStringFromSearchTree(FusedDictionaryAndBuffer<Token>& dict) {
    if (dict.dictionary_size() == dict.max_dictionary_size()) {
        search_tree_.RemoveString(dict.get_oldest_dictionary_full_match());
    }
}

template <std::integral Token,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::FlushData(
    BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<Token>>(
        dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<Token>>(dictionary_and_buffer_);
    auto out_range = AsSubrange(std::forward<decltype(output)>(output));

    if (queued_token_) {
        out_range = FlushQueue(out_range);
        if (queued_token_) {
            return out_range;
        }
    }

    for (size_t i = 0; i < search_tree_.string_size(); ++i) {
        out_range =
            PeformEncodigStep(dict, dict.get_buffer(), std::move(out_range));
        dict.AddEndSymbolToBuffer();
    }
    return out_range;
}

}  // namespace koda
