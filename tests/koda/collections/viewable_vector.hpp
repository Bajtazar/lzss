#pragma once

#include <vector>
#include <string>
#include <ranges>
#include <algorithm>

namespace koda::tests {

template <typename Tp>
struct ViewableVector : public std::vector<Tp> {
    using std::vector<Tp>::vector;

    constexpr operator std::basic_string_view<uint8_t>() const noexcept {
        return {this->data(), this->size()};
    }
};

template <std::ranges::input_range Range>
constexpr ViewableVector<uint8_t> ConvertToString(Range&& range) {
    ViewableVector<uint8_t> result;
    std::ranges::transform(
        std::forward<Range>(range), std::back_inserter(result),
        [](const char token) { return static_cast<uint8_t>(token); });
    if (result.back() == 0) {
        result.pop_back();
    }
    return result;
}

constexpr ViewableVector<uint8_t> operator""_u8(const char* string,
                                                std::size_t size) {
    return ConvertToString(std::ranges::subrange{string, string + size});
}

}
