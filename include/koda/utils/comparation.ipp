#pragma once

namespace koda {

[[nodiscard]] constexpr WeakOrdering OrderCast(
    std::weak_ordering ordering) noexcept {
    if (ordering == 0) {
        return WeakOrdering::kEquivalent;
    } else if (ordering < 0) {
        return WeakOrdering::kLess;
    }
    return WeakOrdering::kGreater;
}

}  // namespace koda