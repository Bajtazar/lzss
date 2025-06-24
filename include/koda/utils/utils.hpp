#pragma once

#include <concepts>
#include <iterator>
#include <ranges>

namespace koda {

template <std::contiguous_iterator TargetIter,
          std::contiguous_iterator SourceIter>
    requires(std::same_as<std::iter_value_t<TargetIter>,
                          std::iter_value_t<std::remove_cvref_t<SourceIter>>> &&
             std::is_trivially_copyable_v<std::iter_value_t<TargetIter>>)
constexpr void MemoryCopy(TargetIter target, SourceIter source, size_t length);

template <std::contiguous_iterator TargetIter,
          std::ranges::contiguous_range SourceRange>
    requires(
        std::same_as<
            std::iter_value_t<TargetIter>,
            std::ranges::range_value_t<std::remove_cvref_t<SourceRange>>> &&
        std::is_trivially_copyable_v<std::iter_value_t<TargetIter>>)
constexpr void MemoryCopy(TargetIter target, SourceRange&& source);

template <std::contiguous_iterator TargetIter,
          std::contiguous_iterator SourceIter>
    requires(std::same_as<std::iter_value_t<TargetIter>,
                          std::iter_value_t<std::remove_cvref_t<SourceIter>>> &&
             std::is_trivially_copyable_v<std::iter_value_t<TargetIter>>)
constexpr void MemoryMove(TargetIter target, SourceIter source, size_t length);

template <std::contiguous_iterator TargetIter,
          std::ranges::contiguous_range SourceRange>
    requires(
        std::same_as<
            std::iter_value_t<TargetIter>,
            std::ranges::range_value_t<std::remove_cvref_t<SourceRange>>> &&
        std::is_trivially_copyable_v<std::iter_value_t<TargetIter>>)
constexpr void MemoryMove(TargetIter target, SourceRange&& source);

}  // namespace koda

#include <koda/utils/utils.tpp>
