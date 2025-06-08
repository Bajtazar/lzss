#pragma once

#include <bit>
#include <ranges>

namespace koda {

template <std::integral Token>
/*static*/ constexpr void TokenTraits<Token>::EncodeToken(
    TokenType token, BitOutputRange auto&& output) {
    std::ranges::copy(LittleEndianInputBitRangeWrapper{std::ranges::subrange{
                          &token, std::next(&token)}},
                      std::ranges::begin(output));
}

template <std::integral Token>
[[nodiscard]] /*static*/ constexpr TokenTraits<Token>::TokenType
TokenTraits<Token>::DecodeToken(BitInputRange auto&& input) {
    TokenType result;
    std::ranges::copy(input | std::views::take(sizeof(Token) * 8),
                      LittleEndianOutputBitIter{&result});
    return result;
}

template <std::integral Token>
[[nodiscard]] /*static*/ constexpr float TokenTraits<Token>::TokenBitSize(
    [[maybe_unused]] TokenType token) {
    return sizeof(Token) * CHAR_BIT;
}

}  // namespace koda
