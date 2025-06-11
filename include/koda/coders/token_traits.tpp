#pragma once

#include <bit>
#include <ranges>

namespace koda {

template <std::integral Token>
/*static*/ constexpr auto TokenTraits<Token>::EncodeToken(
    TokenType token, BitOutputRange auto&& output) {
    auto copy_res =
        std::ranges::copy(std::ranges::subrange{&token, std::next(&token)} |
                              views::LittleEndianInput,
                          std::ranges::begin(output));

    return std::ranges::subrange{std::move(copy_res.out),
                                 std::ranges::end(output)};
}

template <std::integral Token>
[[nodiscard]] /*static*/ constexpr auto TokenTraits<Token>::DecodeToken(
    BitInputRange auto&& input) {
    TokenType result;
    auto copy_res =
        std::ranges::copy(input | std::views::take(sizeof(Token) * 8),
                          LittleEndianOutputBitIter{&result});
    return TokenTraitsDecodingResult{
        std::move(result),
        std::ranges::subrange{std::move(copy_res.in), std::ranges::end(input)}};
}

template <std::integral Token>
[[nodiscard]] /*static*/ constexpr float TokenTraits<Token>::TokenBitSize(
    [[maybe_unused]] TokenType token) {
    return sizeof(Token) * CHAR_BIT;
}

}  // namespace koda
