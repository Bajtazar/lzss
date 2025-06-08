#pragma once

#include <algorithm>
#include <functional>
#include <ranges>

namespace koda {

template <typename Token>
constexpr float DirectEncoder<Token>::TokenBitSize(Token token) const {
    return Traits::TokenBitSize(token);
}

template <typename Token>
constexpr void DirectEncoder<Token>::Decode(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    const auto sentinel = std::ranges::end(input);
    auto output_iter = std::ranges::begin(output);
    const auto output_sent = std::ranges::end(output);
    for (auto iter = std::ranges::begin(input);
         iter != sentinel && output_sent != output_sent;) {
        *output_iter++ =
            Traits::template DecodeToken(std::ranges::subrange{iter, sentinel});
    }
}

template <typename Token>
constexpr void DirectEncoder<Token>::operator()(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    Decode(std::forward<decltype(input)>(input),
           std::forward<decltype(output)>(output));
}

}  // namespace koda
