#pragma once

#include <algorithm>
#include <cstring>
#include <memory>

namespace koda {

template <std::ranges::contiguous_iterator TargetIter,
          std::ranges::contiguous_iterator SourceIter>
    requires(std::same_as<std::iter_value_t<TargetIter>,
                          std::iter_value_t<SourceIter>> &&
             std::is_trivially_copyable_v<std::iter_value_t<TargetIter>>)
constexpr void MemoryCopy(TargetIter&& target, SourceIter&& source,
                          size_t length) {
    if consteval {
        std::ranges::copy(source, std::advance(source, length), target);
    } else {
        std::memcpy(std::to_address(target), std::to_address(source),
                    sizeof(std::iter_value_t<TargetIter>) * length);
    }
}

}  // namespace koda
