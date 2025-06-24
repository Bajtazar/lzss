#pragma once

#include <koda/utils/back_inserter_iterator.hpp>

namespace koda {

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
class LzssDecoder<Token, AuxiliaryDecoder, Allocator>::InitializationView {
   public:
    constexpr explicit InitializationView(size_t look_ahead_size) noexcept
        : look_ahead_size_{look_ahead_size} {}

    class Iterator {
       public:
        using value_type = IMToken;
        using difference_type = std::ptrdiff_t;

        constexpr explicit Iterator(
            InitializationView* parent = nullptr) noexcept
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
            auto& tokens = parent_->tokens_;
            if (auto symbol = token.get_symbol()) {
                tokens.emplace_back(*symbol);
            } else {
                const auto [pos, len] = *token.get_marker();
                const auto old_end = tokens.size();
                tokens.resize(old_end + len);
                MemoryMove(tokens.begin() + old_end, tokens.begin() + pos, len);
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
        InitializationView* parent_;
    };

    [[nodiscard]] constexpr Iterator begin() noexcept { return Iterator{this}; }

    [[nodiscard]] static consteval std::default_sentinel_t end() noexcept {
        return std::default_sentinel;
    }

    SequenceView initial_buffer() {
        return {tokens_.begin(), tokens_.begin() + look_ahead_size_};
    }

    SequenceView tail() {
        return {tokens_.begin() + look_ahead_size_, tokens_.end()};
    }

    const std::vector<Token>& tokens() { return tokens_; }

   private:
    std::vector<Token> tokens_{};
    size_t look_ahead_size_;

    constexpr bool HasFinished() const noexcept {
        return tokens_.size() >= look_ahead_size_;
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
    return CoderResult{std::move(input), std::move(output)};
}

}  // namespace koda
