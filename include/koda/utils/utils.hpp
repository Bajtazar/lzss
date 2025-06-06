#pragma once

#include <concepts>
#include <ranges>

namespace koda {

template <std::ranges::contiguous_range TargetRange,
          std::ranges::contiguous_range SourceRange>
    requires(
        std::same_as<std::ranges::range_value_t<TargetRange>,
                     std::ranges::range_value_t<SourceRange>> &&
        std::is_trivially_copyable_v<std::ranges::range_value_t<TargetRange>>)
constexpr void MemoryCopy(TargetRange&& target, SourceRange&& source,
                          size_t length);

}

#include <koda/utils/utils.tpp>
