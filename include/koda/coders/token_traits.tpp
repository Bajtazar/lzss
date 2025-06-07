#pragma once

#include <ranges>

namespace koda {

template <std::integral Token>
/*static*/ constexpr void TokenTraits<Token>::EncodeToken(
    TokenType token, BitOutputRange auto&& output) {
    std::ranges::copy(LittleEndianOutputBitIter{&token},
                      LittleEndianOutputBitIter{}, std::ranges::begin(output));
}

template <std::integral Token>
[[nodiscard]] constexpr /*TokenType*/ TokenTraits<Token>::DecodeToken(
    BitInputRange auto&& input) {
    TokenType result;
    std::ranges::copy(input | std::views::take(sizeof(Token) * 8),
                      LittleEndianInputIterator{&result});
    return result;
}

}  // namespace koda
