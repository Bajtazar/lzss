#pragma once

#include <koda/utils/back_inserter_iterator.hpp>

namespace koda {

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
template <std::ranges::output_range<Token> RangeTp>
class LzssDecoder<Token, AuxiliaryDecoder, Allocator>::SlidingDecoderView {
   public:
    constexpr explicit SlidingDecoderView(
        FusedDictionaryAndBuffer<Token>& dictionary, RangeTp&& range) noexcept
        : dictionary_{dictionary},
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

        constexpr Iterator& operator=(value_type token) noexcept {
            if (auto symbol = token.get_symbol()) {
                *(parent_->iterator_)++ = *symbol;
                parent_->dictionary_.AddSymbolToBuffer(*symbol);
                return *this;
            }
            const auto [pos, len] = *token.get_marker();

            auto sequence =
                parent_->dictionary_.get_sequence_at_relative_pos(pos, len);

            // @TODO fix when iterator hits sentinel
            for (const auto& element : sequence) {
                *(parent_->iterator_)++ = element;
                parent_->dictionary_.AddSymbolToBuffer(element);
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
    };

    [[nodiscard]] constexpr Iterator begin() noexcept { return Iterator{this}; }

    [[nodiscard]] static consteval std::default_sentinel_t end() noexcept {
        return std::default_sentinel;
    }

    auto remaining_range() {
        return std::ranges::subrange{std::move(iterator_),
                                     std::move(sentinel_)};
    }

   private:
    FusedDictionaryAndBuffer<Token>& dictionary_;
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
    : dictionary_and_buffer_{FusedDictAndBufferInfo{
          dictionary_size, look_ahead_size, std::move(cyclic_buffer_size),
          std::move(allocator)}},
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
    LoadFusedDict();
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
constexpr void
LzssDecoder<Token, AuxiliaryDecoder, Allocator>::LoadFusedDict() {
    auto& info = std::get<FusedDictAndBufferInfo>(dictionary_and_buffer_);

    dictionary_and_buffer_ = FusedDictionaryAndBuffer<Token>{
        info.dictionary_size, info.look_ahead_size,
        std::move(info.cyclic_buffer_size), std::move(info.allocator)};
}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssDecoder<Token, AuxiliaryDecoder, Allocator>::Decode(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<Token>>(
        dictionary_and_buffer_))]];

    SlidingDecoderView decoder_view{
        std::get<FusedDictionaryAndBuffer<Token>>(dictionary_and_buffer_),
        std::forward<decltype(output)>(output)};

    auto result = auxiliary_decoder_.Decode(
        std::forward<decltype(input)>(input), decoder_view);

    return CoderResult{std::move(result.input_range),
                       decoder_view.remaining_range()};
}

}  // namespace koda
