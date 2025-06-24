#pragma once

#include <koda/utils/utils.hpp>

#include <algorithm>
#include <functional>
#include <ranges>

namespace koda {

template <typename Token>
constexpr float DirectEncoder<Token>::TokenBitSize(Token token) const {
    return Traits::TokenBitSize(token);
}

template <typename Token>
constexpr auto DirectEncoder<Token>::Encode(InputRange<Token> auto&& input,
                                            BitOutputRange auto&& output) {
    auto out_range = AsSubrange(std::forward<decltype(output)>(output));
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);
    for (; (input_iter != input_sent) && !out_range.empty(); ++input_iter) {
        out_range = Traits::EncodeToken(*input_iter, out_range);
    }
    return CoderResult{std::move(input_iter), std::move(input_sent),
                       std::move(out_range)};
}

template <typename Token>
constexpr auto DirectEncoder<Token>::Flush(BitOutputRange auto&& output) {
    return std::forward<decltype(output)>(output);
}

}  // namespace koda
