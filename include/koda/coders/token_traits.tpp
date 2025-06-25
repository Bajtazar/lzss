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
    TokenType result[1]{};
    auto view = result | views::LittleEndianOutput;

    auto in_iter = std::ranges::begin(input);
    auto in_sent = std::ranges::end(input);

    auto v_iter = std::ranges::begin(view);
    const auto v_sent = std::ranges::end(view);

    for (; in_iter != in_sent && v_iter != v_sent; ++in_iter, ++v_iter) {
        *v_iter = *in_iter;
    }

    return TokenTraitsDecodingResult{
        std::move(result[0]),
        std::ranges::subrange{std::move(in_iter), std::move(in_sent)}};
}

template <std::integral Token>
[[nodiscard]] /*static*/ constexpr float TokenTraits<Token>::TokenBitSize(
    [[maybe_unused]] TokenType token) {
    return sizeof(Token) * CHAR_BIT;
}

}  // namespace koda
