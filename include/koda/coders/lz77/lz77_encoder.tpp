#pragma once

#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/utils/utils.hpp>

namespace koda {

namespace details {

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>::Lz77EncoderBase(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryEncoder auxiliary_encoder,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    : dictionary_and_buffer_{FusedDictAndBufferInfo{
          dictionary_size, std::move(cyclic_buffer_size)}},
      search_tree_{look_ahead_size, allocator},
      auxiliary_encoder_{std::move(auxiliary_encoder)} {}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>::Lz77EncoderBase(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    requires std::is_default_constructible_v<AuxiliaryEncoder>
    : Lz77EncoderBase{dictionary_size, look_ahead_size, AuxiliaryEncoder{},
                      std::move(cyclic_buffer_size), allocator} {}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
[[nodiscard]] constexpr auto&&
Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>::auxiliary_encoder(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.auxiliary_encoder_);
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto
Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>::InitializeBuffer(
    InputRange<Token> auto&& input) {
    // buffer also stores one suffix symbol that is not used during match lookup
    // but is used to construct an intermediate token for the longest match!
    const size_t buffer_size = 1 + search_tree_.string_size();

    auto [dict_size, cyclic_buffer_size] =
        std::get<FusedDictAndBufferInfo>(dictionary_and_buffer_);

    if constexpr (std::ranges::sized_range<decltype(input)>) {
        dictionary_and_buffer_ = FusedDictionaryAndBuffer{
            dict_size, input | std::views::take(buffer_size),
            std::move(cyclic_buffer_size), search_tree_.get_allocator()};
    } else {
        std::vector<Token> init_view{std::from_range,
                                     input | std::views::take(buffer_size)};

        dictionary_and_buffer_ = FusedDictionaryAndBuffer{
            dict_size, init_view, std::move(cyclic_buffer_size),
            search_tree_.get_allocator()};
    }

    return input | std::views::drop(buffer_size);
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>::FlushQueue(
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
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
constexpr auto
Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>::EncodeIntermediateToken(
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

}  // namespace details

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsSymetrical)
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Flush(
    BitOutputRange auto&& output) {
    return this->auxiliary_encoder_.Flush(
        FlushData(std::forward<decltype(output)>(output)));
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsSymetrical)
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Encode(
    InputRange<Token> auto&& input, BitOutputRange auto&& output) {
    if (std::holds_alternative<typename Base::FusedDictAndBufferInfo>(
            this->dictionary_and_buffer_)) {
        return EncodeData(this->InitializeBuffer(input), output);
    }
    return EncodeData(input, output);
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsSymetrical)
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::EncodeData(
    InputRange<Token> auto&& input, BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<Token>>(
        this->dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<Token>>(this->dictionary_and_buffer_);

    auto out_range = AsSubrange(std::forward<decltype(output)>(output));

    if (this->queued_token_) {
        out_range = this->FlushQueue(out_range);
        if (this->queued_token_) {
            return CoderResult{std::forward<decltype(input)>(input),
                               std::move(out_range)};
        }
    }

    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);
    for (; (input_iter != input_sent) && !out_range.empty(); ++input_iter) {
        auto [buffer, look_ahead] = GetBufferAndLookAhead(dict);
        out_range =
            PeformEncodigStep(dict, buffer, look_ahead, std::move(out_range));
        this->search_tree_.AddString(look_ahead);
        dict.AddSymbolToBuffer(*input_iter);
    }
    return CoderResult{std::move(input_iter), std::move(input_sent),
                       std::move(out_range)};
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsSymetrical)
constexpr auto
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::EncodeTokenOrMatch(
    SequenceView buffer, const Match& match, BitOutputRange auto&& output) {
    // buffer is one symbol longer than the actual look-ahead buffer
    auto suffix_token = buffer[match.match_length];
    IMToken symbol_token{
        suffix_token,
        static_cast<typename IMToken::Position>(match.match_position),
        static_cast<typename IMToken::Length>(match.match_length)};

    this->match_count_ = match.match_length;
    return this->EncodeIntermediateToken(
        std::move(symbol_token), std::forward<decltype(output)>(output));
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsSymetrical)
constexpr auto
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::PeformEncodigStep(
    FusedDictionaryAndBuffer<Token>& dict, SequenceView buffer,
    SequenceView look_ahead, BitOutputRange auto&& output) {
    if (!this->match_count_) {
        auto new_output =
            EncodeTokenOrMatch(buffer, this->search_tree_.FindMatch(look_ahead),
                               std::move(output));
        TryToRemoveStringFromSearchTree(dict);
        return new_output;
    }

    --this->match_count_;
    TryToRemoveStringFromSearchTree(dict);
    return AsSubrange(output);
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsSymetrical)
constexpr void Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::
    TryToRemoveStringFromSearchTree(FusedDictionaryAndBuffer<Token>& dict) {
    if (dict.full()) {
        auto string = dict.get_oldest_dictionary_full_match();
        string.remove_suffix(1);
        this->search_tree_.RemoveString(string);
    }
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsSymetrical)
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::FlushData(
    BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<Token>>(
        this->dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<Token>>(this->dictionary_and_buffer_);
    auto out_range = AsSubrange(std::forward<decltype(output)>(output));

    if (this->queued_token_) {
        out_range = this->FlushQueue(out_range);
        if (this->queued_token_) {
            return out_range;
        }
    }

    // Buffer is one element longer than the search tree match
    for (size_t i = 0; i < dict.max_buffer_size(); ++i) {
        auto [buffer, look_ahead] = GetBufferAndLookAhead(dict);
        out_range =
            PeformEncodigStep(dict, buffer, look_ahead, std::move(out_range));
        dict.AddEndSymbolToBuffer();
    }
    return out_range;
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsSymetrical)
constexpr std::pair<
    typename Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::SequenceView,
    typename Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::SequenceView>
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::GetBufferAndLookAhead(
    FusedDictionaryAndBuffer<Token>& dict) const {
    auto buffer = dict.get_buffer();
    auto look_ahead = buffer;
    look_ahead.remove_suffix(1);
    return {buffer, look_ahead};
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Flush(
    BitOutputRange auto&& output) {
    return this->auxiliary_encoder_.Flush(
        FlushData(std::forward<decltype(output)>(output)));
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Lz77Encoder(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryEncoder auxiliary_encoder,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    : Base{dictionary_size + look_ahead_size + 1, look_ahead_size,
           std::move(auxiliary_encoder), std::move(cyclic_buffer_size),
           allocator} {}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Lz77Encoder(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    requires std::is_default_constructible_v<AuxiliaryEncoder>
    : Base{dictionary_size + look_ahead_size + 1, look_ahead_size,
           std::move(cyclic_buffer_size), allocator} {}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::Encode(
    InputRange<Token> auto&& input, BitOutputRange auto&& output) {
    if (std::holds_alternative<typename Base::FusedDictAndBufferInfo>(
            this->dictionary_and_buffer_)) {
        return EncodeData(this->InitializeBuffer(input), output);
    }
    return EncodeData(input, output);
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::EncodeData(
    InputRange<Token> auto&& input, BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<Token>>(
        this->dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<Token>>(this->dictionary_and_buffer_);

    auto out_range = AsSubrange(std::forward<decltype(output)>(output));

    if (this->queued_token_) {
        out_range = this->FlushQueue(out_range);
        if (this->queued_token_) {
            return CoderResult{std::forward<decltype(input)>(input),
                               std::move(out_range)};
        }
    }

    auto [input_iter, input_sent] =
        PopulateDictionary(std::forward<decltype(input)>(input), dict);

    for (; (input_iter != input_sent) && !out_range.empty(); ++input_iter) {
        auto [buffer, look_ahead] = GetBufferAndLookAhead(dict);
        this->search_tree_.RemoveString(look_ahead);
        AddStringToSearchTree(dict);
        out_range =
            PeformEncodigStep(dict, buffer, look_ahead, std::move(out_range));

        dict.AddSymbolToBuffer(*input_iter);
    }
    return CoderResult{std::move(input_iter), std::move(input_sent),
                       std::move(out_range)};
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr auto
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::PopulateDictionary(
    InputRange<Token> auto&& input, FusedDictionaryAndBuffer<Token>& dict) {
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);

    // Asymetrical mode finds repeatitions in the future so entire dictionary
    // has to be populated before the algorithm even starts working
    for (; !dict.full() && (input_iter != input_sent); ++input_iter) {
        AddStringToSearchTree(dict);
        dict.AddSymbolToBuffer(*input_iter);
    }

    return std::pair{std::move(input_iter), std::move(input_sent)};
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr auto
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::PeformEncodigStep(
    FusedDictionaryAndBuffer<Token>& dict, SequenceView buffer,
    SequenceView look_ahead, BitOutputRange auto&& output) {
    if (!this->match_count_) {
        auto new_output = EncodeTokenOrMatch(
            dict, buffer, this->search_tree_.FindMatch(look_ahead),
            std::move(output));
        return new_output;
    }

    --this->match_count_;
    return AsSubrange(output);
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr auto
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::EncodeTokenOrMatch(
    FusedDictionaryAndBuffer<Token>& dict, SequenceView buffer, Match&& match,
    BitOutputRange auto&& output) {
    // buffer is one symbol longer than the actual look-ahead buffer
    // calculate the match end position since decoding will be performed from
    // the last symbol to the first one
    auto match_end_position = match.match_position + match.match_length;
    match.match_position =
        match_end_position >= dict.dictionary_size()
            ? 0
            : dict.dictionary_size() - match_end_position - 1;
    match.match_length = match_end_position >= dict.dictionary_size()
                             ? dict.dictionary_size() - match.match_position - 1
                             : match.match_length;

    auto suffix_token = buffer[match.match_length];
    IMToken symbol_token{
        suffix_token,
        static_cast<typename IMToken::Position>(match.match_position),
        static_cast<typename IMToken::Length>(match.match_length)};

    this->match_count_ = match.match_length;
    return this->EncodeIntermediateToken(
        std::move(symbol_token), std::forward<decltype(output)>(output));
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr std::pair<
    typename Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::SequenceView,
    typename Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::SequenceView>
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::GetBufferAndLookAhead(
    FusedDictionaryAndBuffer<Token>& dict) const {
    auto buffer = dict.get_oldest_dictionary_full_match();
    auto look_ahead = buffer;
    look_ahead.remove_suffix(1);
    return {buffer, look_ahead};
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr void
Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::AddStringToSearchTree(
    FusedDictionaryAndBuffer<Token>& dict) {
    auto buffer = dict.get_buffer();
    this->search_tree_.AddString(
        {buffer.begin(),
         std::next(buffer.begin(), this->search_tree_.string_size())});
}

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
constexpr auto Lz77Encoder<Token, AuxiliaryEncoder, Allocator>::FlushData(
    BitOutputRange auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<Token>>(
        this->dictionary_and_buffer_))]];

    auto& dict =
        std::get<FusedDictionaryAndBuffer<Token>>(this->dictionary_and_buffer_);
    auto out_range = AsSubrange(std::forward<decltype(output)>(output));

    if (this->queued_token_) {
        out_range = this->FlushQueue(out_range);
        if (this->queued_token_) {
            return out_range;
        }
    }

    // Remove front buffer
    for (size_t i = 0; !dict.full() && !dict.get_buffer().empty(); ++i) {
        AddStringToSearchTree(dict);
        dict.AddEndSymbolToBuffer();
    }

    // Buffer is one element longer than the search tree match
    while (!dict.empty()) {
        auto [buffer, look_ahead] = GetBufferAndLookAhead(dict);
        this->search_tree_.RemoveString(look_ahead);
        out_range =
            PeformEncodigStep(dict, buffer, look_ahead, std::move(out_range));
        dict.AddEndSymbolToBuffer();
    }
    return out_range;
}

}  // namespace koda
