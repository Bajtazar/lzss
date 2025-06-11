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
        std::tie(*output_iter++, input_range) =
            Traits::template DecodeToken(input_range);
    }
    return std::ranges::subrange{std::move(output_iter),
                                 std::ranges::end(output)};
}

template <typename Token>
constexpr auto DirectDecoder<Token>::operator()(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    auto decoding_res = Decode(std::forward<decltype(input)>(input),
                               std::forward<decltype(output)>(output));
    auto iter = std::ranges::begin(decoding_res);
    iter.Flush();
    return std::ranges::subrange{std::move(iter),
                                 std::ranges::end(decoding_res)};
}

}  // namespace koda
