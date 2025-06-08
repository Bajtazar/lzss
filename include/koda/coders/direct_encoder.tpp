#pragma once

#include <algorithm>
#include <functional>
#include <ranges>

namespace koda {

template <typename Token>
constexpr float DirectEncoder<Token>::TokenBitSize(Token token) const {
    return 0;
}

template <typename Token>
constexpr void DirectEncoder<Token>::Encode(InputRange<Token> auto&& input,
                                            BitOutputRange auto&& output) {
    std::ranges::for_each(
        std::forward<decltype(input)>(input),
        std::bind_back(Traits::template EncodeToken<decltype(output)>,
                       std::forward<decltype(output)>(output)));
}

template <typename Token>
constexpr void DirectEncoder<Token>::Flush(
    [[maybe_unused]] BitOutputRange auto&& output) {}

template <typename Token>
constexpr void DirectEncoder<Token>::operator()(InputRange<Token> auto&& input,
                                                BitOutputRange auto&& output) {
    Encode(std::forward<decltype(input)>(input),
           std::forward<decltype(output)>(output));
}

}  // namespace koda
