#pragma once

namespace koda::ranges {

template <typename Tp, std::output_iterator<Tp> IterTp,
          std::sentinel_for<IterTp> SentTp>
[[nodiscard]] constexpr bool operator==(
    const OutputTakeIterator<Tp, IterTp>& left,
    const OutputTakeSentinel<IterTp, SentTp>& right) noexcept {
    return (left.base() == right.base()) || (left.counter() == right.counter());
}

template <typename Tp, std::output_iterator<Tp> IterTp,
          std::sentinel_for<IterTp> SentTp>
[[nodiscard]] constexpr bool operator==(
    const OutputTakeSentinel<IterTp, SentTp>& left,
    const OutputTakeIterator<Tp, IterTp>& right) noexcept {
    return (left.base() == right.base()) || (left.counter() == right.counter());
}

template <typename IterTp, std::sentinel_for<IterTp> SentinelTp>
constexpr OutputTakeSentinel<IterTp, SentinelTp>::OutputTakeSentinel(
    SentinelTp sentinel,
    size_t limit) noexcept(std::is_nothrow_move_constructible_v<IterTp>)
    : sentinel_{std::move(sentinel)}, counter_{limit} {}

template <typename IterTp, std::sentinel_for<IterTp> SentinelTp>
[[nodiscard]] constexpr auto&& OutputTakeSentinel<IterTp, SentinelTp>::base(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.sentinel_);
}

template <typename IterTp, std::sentinel_for<IterTp> SentinelTp>
[[nodiscard]] constexpr size_t OutputTakeSentinel<IterTp, SentinelTp>::counter()
    const noexcept {
    return counter_;
}

}  // namespace koda::ranges
