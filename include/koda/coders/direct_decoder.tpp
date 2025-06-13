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
    BitInputRange auto&& input, size_t stream_length,
    std::ranges::output_range<Token> auto&& output) {
    auto output_iter = std::ranges::begin(output);

    std::ranges::subrange input_range{std::ranges::begin(input),
                                      std::ranges::end(input)};
    auto output_sentinel = std::ranges::end(output);
    size_t idx = 0;
    for (size_t idx = 0; (idx < stream_length) && (!input_range.empty()) &&
                         (output_iter != output_sentinel);
         ++idx, ++output_iter) {
        auto [token, updated_input_range] = Traits::DecodeToken(input_range);
        *output_iter = std::move(token);
        input_range = std::move(updated_input_range);
    }
    return DecoderResult{std::ranges::subrange{std::move(output_iter),
                                               std::move(output_sentinel)},
                         stream_length - idx};
}

template <typename Token>
constexpr auto DirectDecoder<Token>::Initialize(BitInputRange auto&& input) {
    return DecoderResult{std::forward<decltype(input)>(input), 0};
}

}  // namespace koda
