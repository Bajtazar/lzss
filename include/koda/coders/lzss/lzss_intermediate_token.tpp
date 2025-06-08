#pragma once

namespace koda {

template <std::integral InputToken>
constexpr LzssIntermediateToken<InputToken>::LzssIntermediateToken(
    InputToken symbol) noexcept
    : symbol_{symbol}, holds_distance_match_pair_{false} {}

template <std::integral InputToken>
constexpr LzssIntermediateToken<InputToken>::LzssIntermediateToken(
    size_t match_position, size_t match_length, uint16_t position_bitlength,
    uint16_t length_bitlength) noexcept
    : repeatition_marker_{match_position, match_length, position_bitlength,
                          length_bitlength},
      holds_distance_match_pair_{true} {}

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
/*static*/ constexpr void
TokenTraits<LzssIntermediateToken<InputToken>>::EncodeToken(
    TokenType token, BitOutputRange auto&& output) {
    auto iter = std::ranges::begin(output);
    if (auto symbol = token.get_symbol()) {
        *iter = 0;
        ++iter;
        return TokenTraits<typename LzssIntermediateToken<InputToken>::Symbol>::
            EncodeToken(*symbol, std::forward<decltype(output)>(output));
    }
    auto [position, length, pos_bitlen, len_bitlen] = *token.get_marker();
    *iter = 1;
    ++iter;
    std::ranges::copy(LittleEndianInputBitRangeWrapper{std::ranges::subrange{
                          &position, std::next(&position)}} |
                          std::views::take(pos_bitlen),
                      std::ranges::begin(output));
    std::ranges::copy(LittleEndianInputBitRangeWrapper{std::ranges::subrange{
                          &length, std::next(&length)}} |
                          std::views::take(len_bitlen),
                      std::ranges::begin(output));
}

template <std::integral InputToken>
[[nodiscard]] /*static*/ constexpr TokenTraits<
    LzssIntermediateToken<InputToken>>::TokenType
TokenTraits<LzssIntermediateToken<InputToken>>::DecodeToken(
    BitInputRange auto&& input) {
    // TokenType result;
    // std::ranges::copy(input | std::views::take(sizeof(Token) * 8),
    //                   LittleEndianOutputBitIter{&result});
    // return result;
}

}  // namespace koda
