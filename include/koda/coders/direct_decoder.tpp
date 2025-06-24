#pragma once

#include <koda/utils/utils.hpp>

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
    auto input_range = AsSubrange(std::forward<decltype(input)>(input));

    auto output_iter = std::ranges::begin(output);
    auto output_sentinel = std::ranges::end(output);
    for (; !input_range.empty() && (output_iter != output_sentinel);
         ++output_iter) {
        auto [token, updated_input_range] = Traits::DecodeToken(input_range);
        *output_iter = std::move(token);
        input_range = std::move(updated_input_range);
    }
    return CoderResult{std::move(input_range), std::move(output_iter),
                       std::move(output_sentinel)};
}

template <typename Token>
constexpr auto DirectDecoder<Token>::Initialize(BitInputRange auto&& input) {
    return std::forward<decltype(input)>(input);
}

}  // namespace koda
