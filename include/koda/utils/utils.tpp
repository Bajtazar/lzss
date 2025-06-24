#pragma once

#include <algorithm>
#include <cstring>
#include <memory>

namespace koda {

template <std::contiguous_iterator TargetIter,
          std::contiguous_iterator SourceIter>
    requires(std::same_as<std::iter_value_t<TargetIter>,
                          std::iter_value_t<std::remove_cvref_t<SourceIter>>> &&
             std::is_trivially_copyable_v<std::iter_value_t<TargetIter>>)
constexpr void MemoryCopy(TargetIter target, SourceIter source, size_t length) {
    if consteval {
        std::ranges::copy(source, std::next(source, length), target);
    } else {
        std::memcpy(std::to_address(target), std::to_address(source),
                    sizeof(std::iter_value_t<TargetIter>) * length);
    }
}

template <std::contiguous_iterator TargetIter,
          std::ranges::contiguous_range SourceRange>
    requires(
        std::same_as<
            std::iter_value_t<TargetIter>,
            std::ranges::range_value_t<std::remove_cvref_t<SourceRange>>> &&
        std::is_trivially_copyable_v<std::iter_value_t<TargetIter>>)
constexpr void MemoryCopy(TargetIter target, SourceRange&& source) {
    return MemoryCopy(std::move(target), std::ranges::begin(source),
                      std::ranges::size(source));
}

template <std::contiguous_iterator TargetIter,
          std::contiguous_iterator SourceIter>
    requires(std::same_as<std::iter_value_t<TargetIter>,
                          std::iter_value_t<std::remove_cvref_t<SourceIter>>> &&
             std::is_trivially_copyable_v<std::iter_value_t<TargetIter>>)
constexpr void MemoryMove(TargetIter target, SourceIter source, size_t length) {
    if consteval {
        std::ranges::copy(source, std::next(source, length), target);
    } else {
        std::memmove(std::to_address(target), std::to_address(source),
                     sizeof(std::iter_value_t<TargetIter>) * length);
    }
}

template <std::contiguous_iterator TargetIter,
          std::ranges::contiguous_range SourceRange>
    requires(
        std::same_as<
            std::iter_value_t<TargetIter>,
            std::ranges::range_value_t<std::remove_cvref_t<SourceRange>>> &&
        std::is_trivially_copyable_v<std::iter_value_t<TargetIter>>)
constexpr void MemoryMove(TargetIter target, SourceRange&& source) {
    return MemoryMove(std::move(target), std::ranges::begin(source),
                      std::ranges::size(source));
}

template <std::ranges::range RangeTp>
constexpr auto AsSubrange(RangeTp&& range) {
    return std::ranges::subrange{std::ranges::begin(range),
                                 std::ranges::end(range)};
}

}  // namespace koda
