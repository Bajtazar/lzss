#pragma once

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

namespace koda::tests {

template <typename Tp>
struct ViewableVector : public std::vector<Tp> {
    using std::vector<Tp>::vector;

    constexpr operator std::basic_string_view<uint8_t>() const noexcept;
};

template <std::ranges::input_range Range>
constexpr ViewableVector<uint8_t> ConvertToString(Range&& range);

constexpr ViewableVector<uint8_t> operator""_u8(const char* string,
                                                std::size_t size);

}  // namespace koda::tests

#include <koda/tests/viewable_vector.tpp>
