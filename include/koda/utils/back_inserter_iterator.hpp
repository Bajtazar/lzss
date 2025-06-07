#pragma once

#include <iterator>

namespace koda {

template <typename ContainterTp>
struct BackInserterIterator : public std::back_insert_iterator<ContainterTp> {
    using std::back_insert_iterator<ContainterTp>::back_insert_iterator;

    [[nodiscard]] friend constexpr bool operator==(
        [[maybe_unused]] BackInserterIterator const& left,
        [[maybe_unused]] std::default_sentinel_t sentinel) noexcept {
        return true;
    }

    [[nodiscard]] friend constexpr bool operator==(
        [[maybe_unused]] std::default_sentinel_t sentinel,
        [[maybe_unused]] BackInserterIterator const& right) noexcept {
        return true;
    }
};

}  // namespace koda
