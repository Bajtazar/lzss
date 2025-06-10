#pragma once

#include <koda/utils/type_dummies.hpp>

#include <concepts>

namespace koda {

template <typename Range, typename Tp>
concept InputRange = std::ranges::input_range<Range> &&
    std::same_as<std::ranges::range_value_t<Range>, Tp>;

}
