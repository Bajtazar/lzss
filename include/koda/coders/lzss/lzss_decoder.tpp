#pragma once

#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/utils.hpp>

namespace koda {

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
template <std::ranges::output_range<Token> RangeTp>
class LzssDecoder<Token, AuxiliaryDecoder, Allocator>::SlidingDecoderView {
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
            if (auto symbol = token.get_symbol()) {
                *(parent_->iterator_)++ = *symbol;
                parent_->dictionary_.AddSymbolToBuffer(*symbol);
                return *this;
            }
            CopySequenceToDictionary(std::move(token));
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
            auto [position, length] = *token.get_marker();

            auto sequence = parent_->dictionary_.get_sequence_at_relative_pos(
                position, length);

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
            if (length) {
                [[assume(!(parent_->cached_sequence_))]];
                parent_->cached_sequence_ = CachedSequence{position, length};
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
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr LzssDecoder<Token, AuxiliaryDecoder, Allocator>::LzssDecoder(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryDecoder auxiliary_decoder,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    : dictionary_{CalculateDictionarySize(dictionary_size, look_ahead_size),
                  look_ahead_size, std::move(cyclic_buffer_size),
                  std::move(allocator)},
      auxiliary_decoder_{std::move(auxiliary_decoder)} {}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr LzssDecoder<Token, AuxiliaryDecoder, Allocator>::LzssDecoder(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    requires std::is_default_constructible_v<AuxiliaryDecoder>
    : LzssDecoder{dictionary_size, look_ahead_size, AuxiliaryDecoder{},
                  std::move(cyclic_buffer_size), std::move(allocator)} {}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssDecoder<Token, AuxiliaryDecoder, Allocator>::Initialize(
    BitInputRange auto&& input) {
    return auxiliary_decoder_.Initialize(std::forward<decltype(input)>(input));
}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
[[nodiscard]] constexpr auto&&
LzssDecoder<Token, AuxiliaryDecoder, Allocator>::auxiliary_decoder(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.auxiliary_decoder_);
}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssDecoder<Token, AuxiliaryDecoder, Allocator>::Decode(
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
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto
LzssDecoder<Token, AuxiliaryDecoder, Allocator>::ProcessCachedSequence(
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

    if (!cache.length) {
        cached_sequence_ = std::nullopt;
    }

    return std::ranges::subrange{std::move(out_iter), std::move(out_sent)};
}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssDecoder<Token, AuxiliaryDecoder, Allocator>::ProcessData(
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

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
/*static*/ constexpr size_t
LzssDecoder<Token, AuxiliaryDecoder, Allocator>::CalculateDictionarySize(
    size_t dictionary_size, size_t look_ahead_size) {
    if (dictionary_size < look_ahead_size) [[unlikely]] {
        throw std::logic_error{std::format(
            "Dictionary ({}) cannot be smaller than the buffer size ({})",
            dictionary_size, look_ahead_size)};
    }
    return dictionary_size - look_ahead_size;
}

}  // namespace koda
