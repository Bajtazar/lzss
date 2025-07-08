#pragma once

#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/utils.hpp>

namespace koda {

template <std::integral Token,
          SizeAwareDecoder<Lz77IntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
template <std::ranges::output_range<Token> RangeTp>
class Lz77Decoder<Token, AuxiliaryDecoder, Allocator>::SlidingDecoderView {
   public:
    constexpr explicit SlidingDecoderView(
        FusedDictionaryAndBuffer<Token>& dictionary,
        std::optional<CachedSequence>& cached_sequence,
        RangeTp&& range) noexcept
        : dictionary_{dictionary},
          cached_sequence_{cached_sequence},
          iterator_{std::ranges::begin(range)},
          sentinel_{std::ranges::end(range)} {}

    class Iterator {
       public:
        using value_type = IMToken;
        using difference_type = std::ptrdiff_t;

        constexpr explicit Iterator(
            SlidingDecoderView* parent = nullptr) noexcept
            : parent_{parent} {}

        [[nodiscard]] friend constexpr bool operator==(
            Iterator const& left,
            [[maybe_unused]] std::default_sentinel_t sentinel) noexcept {
            return left.parent_->HasFinished();
        }

        [[nodiscard]] friend constexpr bool operator==(
            [[maybe_unused]] std::default_sentinel_t sentinel,
            Iterator const& right) noexcept {
            return right.parent_->HasFinished();
        }

        constexpr Iterator& operator=(value_type token) {
            CopySequenceToDictionary(std::move(token));
            if (parent_->iterator_ != parent_->sentinel_) {
                *parent_->iterator_++ = token.suffix_symbol();
            }
            return *this;
        }

        [[nodiscard]] constexpr Iterator& operator*(void) noexcept {
            return *this;
        }

        constexpr Iterator& operator++() noexcept { return *this; }

        [[nodiscard]] constexpr Iterator& operator++(int) noexcept {
            return *this;
        }

       private:
        SlidingDecoderView* parent_;

        constexpr void CopySequenceToDictionary(value_type token) {
            const auto position = token.match_position();
            const auto length = token.match_length();

            const auto sequence =
                parent_->dictionary_.get_sequence_at_relative_pos(position,
                                                                  length);

            auto& iter = parent_->iterator_;
            const auto& sent = parent_->sentinel_;
            for (size_t i = 0; i < sequence.size() && iter != sent;
                 ++iter, --length, ++i) {
                *iter = sequence[i];
                if (!parent_->dictionary_.AddSymbolToBuffer(sequence[i])) {
                    // Dictionary is not yet full so position pointer has to be
                    // advanced manually
                    ++position;
                }
            }
            if (iter == sent) {
                [[assume(!(parent_->cached_sequence_))]];
                parent_->cached_sequence_ =
                    CachedSequence{position, length, token.suffix_symbol()};
            }
        }
    };

    [[nodiscard]] constexpr Iterator begin() noexcept { return Iterator{this}; }

    [[nodiscard]] static consteval std::default_sentinel_t end() noexcept {
        return std::default_sentinel;
    }

    constexpr auto remaining_range() {
        return std::ranges::subrange{std::move(iterator_),
                                     std::move(sentinel_)};
    }

   private:
    FusedDictionaryAndBuffer<Token>& dictionary_;
    std::optional<CachedSequence>& cached_sequence_;
    std::ranges::iterator_t<RangeTp> iterator_;
    std::ranges::sentinel_t<RangeTp> sentinel_;

    constexpr bool HasFinished() const noexcept {
        return iterator_ == sentinel_;
    }
};

template <std::integral Token,
          SizeAwareDecoder<Lz77IntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
constexpr Lz77Decoder<Token, AuxiliaryDecoder, Allocator>::Lz77Decoder(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryDecoder auxiliary_decoder,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    : dictionary_{dictionary_size, look_ahead_size,
                  std::move(cyclic_buffer_size), std::move(allocator)},
      auxiliary_decoder_{std::move(auxiliary_decoder)} {}

template <std::integral Token,
          SizeAwareDecoder<Lz77IntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
constexpr Lz77Decoder<Token, AuxiliaryDecoder, Allocator>::Lz77Decoder(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    requires std::is_default_constructible_v<AuxiliaryDecoder>
    : Lz77Decoder{dictionary_size, look_ahead_size, AuxiliaryDecoder{},
                  std::move(cyclic_buffer_size), std::move(allocator)} {}

template <std::integral Token,
          SizeAwareDecoder<Lz77IntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
constexpr auto Lz77Decoder<Token, AuxiliaryDecoder, Allocator>::Initialize(
    BitInputRange auto&& input) {
    return auxiliary_decoder_.Initialize(std::forward<decltype(input)>(input));
}

template <std::integral Token,
          SizeAwareDecoder<Lz77IntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
[[nodiscard]] constexpr auto&&
Lz77Decoder<Token, AuxiliaryDecoder, Allocator>::auxiliary_decoder(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.auxiliary_decoder_);
}

template <std::integral Token,
          SizeAwareDecoder<Lz77IntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
constexpr auto Lz77Decoder<Token, AuxiliaryDecoder, Allocator>::Decode(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    if (cached_sequence_) {
        auto new_output =
            ProcessCachedSequence(std::forward<decltype(output)>(output));
        if (cached_sequence_) {
            return CoderResult{std::forward<decltype(input)>(input),
                               std::move(new_output)};
        }
        return ProcessData(std::forward<decltype(input)>(input),
                           std::move(new_output));
    }

    return ProcessData(std::forward<decltype(input)>(input),
                       std::forward<decltype(output)>(output));
}

template <std::integral Token,
          SizeAwareDecoder<Lz77IntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
constexpr auto
Lz77Decoder<Token, AuxiliaryDecoder, Allocator>::ProcessCachedSequence(
    std::ranges::output_range<Token> auto&& output) {
    auto& cache = *cached_sequence_;

    auto out_iter = std::ranges::begin(output);
    auto out_sent = std::ranges::end(output);

    auto sequence =
        dictionary_.get_sequence_at_relative_pos(cache.position, cache.length);

    for (size_t i = 0; out_iter != out_sent && cache.length;
         ++out_iter, --cache.length, ++i) {
        *out_iter = sequence[i];
        if (!dictionary_.AddSymbolToBuffer(sequence[i])) {
            // Dictionary is not yet full so position pointer has to be advanced
            // manually
            ++cache.position;
        }
    }

    if (out_iter != out_sent) {
        *out_iter++ = cache.suffix;
        dictionary_.AddSymbolToBuffer(cache.suffix);
        cached_sequence_ = std::nullopt;
    }

    return std::ranges::subrange{std::move(out_iter), std::move(out_sent)};
}

template <std::integral Token,
          SizeAwareDecoder<Lz77IntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
constexpr auto Lz77Decoder<Token, AuxiliaryDecoder, Allocator>::ProcessData(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    SlidingDecoderView decoder_view{
        dictionary_, cached_sequence_,
        std::views::all(std::forward<decltype(output)>(output))};

    auto result = auxiliary_decoder_.Decode(
        std::forward<decltype(input)>(input), decoder_view);

    return CoderResult{std::move(result.input_range),
                       decoder_view.remaining_range()};
}

}  // namespace koda
