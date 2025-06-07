#pragma once

#include <algorithm>
#include <ranges>

namespace koda {

template <typename Token>
constexpr void IdentityEncoder<Token>::Encode(
    InputRange<InputToken> auto&& input, BitOutputRange auto&& output) {
    std::ranges::for_each(
        std::forward<decltype(input)>(input),
        std::bind_back(Traits::EncodeToken,
                       std::forward<decltype(output)>(output)));
}

template <typename Token>
constexpr void IdentityEncoder<Token>::Flush(
    [[maybe_unused]] BitOutputRange auto&& output) {}

template <typename Token>
constexpr void IdentityEncoder<Token>::operator()(
    InputRange<InputToken> auto&& input, BitOutputRange auto&& output) {
    Encode(std::forward<decltype(input)>(input),
           std::forward<decltype(output)>(output));
}

}  // namespace koda
