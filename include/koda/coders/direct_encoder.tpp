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
constexpr auto DirectEncoder<Token>::Encode(InputRange<Token> auto&& input,
                                            BitOutputRange auto&& output) {
    std::ranges::subrange out_range{std::ranges::begin(output),
                                    std::ranges::end(output)};
    for (const auto& token : input) {
        out_range = Traits::EncodeToken(token, out_range);
    }
    return out_range;
}

template <typename Token>
constexpr auto DirectEncoder<Token>::Flush(BitOutputRange auto&& output) {
    return std::forward<decltype(output)>(output);
}

}  // namespace koda
