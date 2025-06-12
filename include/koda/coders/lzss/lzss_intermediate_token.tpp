#pragma once

namespace koda {

template <std::integral InputToken>
constexpr LzssIntermediateToken<InputToken>::LzssIntermediateToken(
    InputToken symbol) noexcept
    : symbol_{symbol}, holds_distance_match_pair_{false} {}

template <std::integral InputToken>
constexpr LzssIntermediateToken<InputToken>::LzssIntermediateToken(
    uint32_t match_position, uint16_t match_length) noexcept
    : repeatition_marker_{match_position, match_length},
      holds_distance_match_pair_{true} {}

template <std::integral InputToken>
[[nodiscard]] constexpr std::partial_ordering
LzssIntermediateToken<InputToken>::operator<=>(
    const LzssIntermediateToken& right) const noexcept {
    if (auto symbol = get_symbol()) {
        if (auto other = right.get_symbol()) {
            return *symbol <=> *other;
        }
        return std::partial_ordering::unordered;
    }
    if (auto other = right.get_marker()) {
        return *get_marker() <=> *other;
    }
    return std::partial_ordering::unordered;
}

template <std::integral InputToken>
[[nodiscard]] constexpr bool LzssIntermediateToken<InputToken>::operator==(
    const LzssIntermediateToken& right) const noexcept {
    if (auto symbol = get_symbol()) {
        if (auto other = right.get_symbol()) {
            return *symbol == *other;
        }
        return false;
    }
    if (auto other = right.get_marker()) {
        return *get_marker() == *other;
    }
    return false;
}

template <std::integral InputToken>
[[nodiscard]] constexpr bool LzssIntermediateToken<InputToken>::holds_symbol()
    const noexcept {
    return holds_distance_match_pair_ == false;
}

template <std::integral InputToken>
[[nodiscard]] constexpr bool LzssIntermediateToken<InputToken>::holds_marker()
    const noexcept {
    return holds_distance_match_pair_ == true;
}

template <std::integral InputToken>
[[nodiscard]] constexpr std::optional<InputToken>
LzssIntermediateToken<InputToken>::get_symbol() const noexcept {
    if (holds_symbol()) {
        return symbol_;
    }
    return std::nullopt;
}

template <std::integral InputToken>
[[nodiscard]] constexpr std::optional<
    typename LzssIntermediateToken<InputToken>::RepeatitionMarker>
LzssIntermediateToken<InputToken>::get_marker() const noexcept {
    if (holds_marker()) {
        return repeatition_marker_;
    }
    return std::nullopt;
}

template <std::integral InputToken>
/*static*/ constexpr auto
TokenTraits<LzssIntermediateToken<InputToken>>::EncodeToken(
    TokenType token, BitOutputRange auto&& output) {
    auto iter = std::ranges::begin(output);
    if (auto symbol = token.get_symbol()) {
        *iter = 0;
        ++iter;
        return TokenTraits<typename LzssIntermediateToken<InputToken>::Symbol>::
            EncodeToken(*symbol, std::forward<decltype(output)>(output));
    }
    auto [position, length] = *token.get_marker();
    *iter = 1;
    ++iter;
    auto pos_range = TokenTraits<uint32_t>::EncodeToken(
        position, std::forward<decltype(output)>(output));
    return TokenTraits<uint16_t>::EncodeToken(length, std::move(pos_range));
}

template <std::integral InputToken>
[[nodiscard]] /*static*/ constexpr auto
TokenTraits<LzssIntermediateToken<InputToken>>::DecodeToken(
    BitInputRange auto&& input) {
    auto iter = std::ranges::begin(input);

    if (auto value = *iter; (++iter, value)) {
        auto [position, pos_range] = TokenTraits<uint32_t>::DecodeToken(
            std::forward<decltype(input)>(input));
        auto [length, len_range] =
            TokenTraits<uint16_t>::DecodeToken(std::move(pos_range));

        return TokenTraitsDecodingResult{
            LzssIntermediateToken<InputToken>{position, length},
            std::move(len_range)};
    }
    auto [token, range] =
        TokenTraits<typename LzssIntermediateToken<InputToken>::Symbol>::
            DecodeToken(std::forward<decltype(input)>(input));

    return TokenTraitsDecodingResult{
        LzssIntermediateToken<InputToken>{std::move(token)}, std::move(range)};
}

template <std::integral InputToken>
[[nodiscard]] /*static*/ constexpr float
TokenTraits<LzssIntermediateToken<InputToken>>::TokenBitSize(TokenType token) {
    if (auto symbol = token.get_symbol()) {
        return TokenTraits<typename LzssIntermediateToken<InputToken>::Symbol>::
            TokenBitSize(*symbol);
    }
    auto [position, length] = *token.get_marker();
    return static_cast<float>(TokenTraits<uint32_t>::TokenBitSize(position) +
                              TokenTraits<uint16_t>::TokenBitSize(length)) /
           length;
}

}  // namespace koda
