#pragma once

#include <cinttypes>
#include <compare>

namespace koda {

enum class WeakOrdering : uint8_t { kEquivalent = 0, kLess = 1, kGreater = 2 };

[[nodiscard]] constexpr WeakOrdering OrderCast(
    std::weak_ordering ordering) noexcept;

}  // namespace koda

#include <koda/utils/comparation.ipp>
