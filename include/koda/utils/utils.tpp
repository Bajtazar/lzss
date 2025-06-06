#pragma once

#include <algorithm>
#include <cstring>
#include <memory>

namespace koda {

template <std::ranges::contiguous_range TargetRange,
          std::ranges::contiguous_range SourceRange>
    requires(
        std::same_as<std::ranges::range_value_t<TargetRange>,
                     std::ranges::range_value_t<SourceRange>> &&
        std::is_trivially_copyable_v<std::ranges::range_value_t<TargetRange>>)
constexpr void MemoryCopy(TargetRange&& target, SourceRange&& source,
                          size_t length) {
    if consteval {
        std::ranges::copy(source | std::views::take(length), target);
    } else {
        std::memcpy(std::ranges::data(target), std::ranges::data(source),
                    sizeof(std::ranges::range_value_t<TargetRange>) * length);
    }
}

}  // namespace koda
