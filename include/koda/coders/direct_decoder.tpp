#pragma once

#include <algorithm>
#include <functional>
#include <ranges>

namespace koda {

template <typename Token>
constexpr float DirectDecoder<Token>::TokenBitSize(Token token) const {
    return Traits::TokenBitSize(token);
}

template <typename Token>
constexpr auto DirectDecoder<Token>::Decode(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    auto output_iter = std::ranges::begin(output);

    std::ranges::subrange input_range{std::ranges::begin(input),
                                      std::ranges::end(input)};
    while (std::ranges::begin(input_range) != std::ranges::end(input_range)) {
        auto [token, updated_input_range] = Traits::DecodeToken(input_range);
        *output_iter++ = std::move(token);
        input_range = std::move(updated_input_range);
    }
    return std::ranges::subrange{std::move(output_iter),
                                 std::ranges::end(output)};
}

}  // namespace koda
