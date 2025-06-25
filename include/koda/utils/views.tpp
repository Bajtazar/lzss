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

template <typename Tp, std::output_iterator<Tp> IterTp>
constexpr OutputTakeIterator<Tp, IterTp>::OutputTakeIterator(
    IterTp iterator,
    size_t counter) noexcept(std::is_nothrow_move_constructible_v<IterTp>)
    : iterator_{std::move(iterator)}, counter_{counter} {}

template <typename Tp, std::output_iterator<Tp> IterTp>
constexpr OutputTakeIterator<Tp, IterTp>&
OutputTakeIterator<Tp, IterTp>::operator=(value_type value) {
    *iterator_ = std::move(value);
    return *this;
}

template <typename Tp, std::output_iterator<Tp> IterTp>
[[nodiscard]] constexpr OutputTakeIterator<Tp, IterTp>&
OutputTakeIterator<Tp, IterTp>::operator*() noexcept {
    return *this;
}

template <typename Tp, std::output_iterator<Tp> IterTp>
constexpr OutputTakeIterator<Tp, IterTp>&
OutputTakeIterator<Tp, IterTp>::operator++() noexcept {
    ++iterator_;
    ++counter_;
    return *this;
}

template <typename Tp, std::output_iterator<Tp> IterTp>
[[nodiscard]] constexpr OutputTakeIterator<Tp, IterTp>
OutputTakeIterator<Tp, IterTp>::operator++(int) noexcept {
    return OutputTakeIterator{iterator_++, counter_++};
}

template <typename Tp, std::output_iterator<Tp> IterTp>
[[nodiscard]] constexpr auto&& OutputTakeIterator<Tp, IterTp>::base(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.iterator_);
}

template <typename Tp, std::output_iterator<Tp> IterTp>
[[nodiscard]] constexpr size_t OutputTakeIterator<Tp, IterTp>::counter()
    const noexcept {
    return counter_;
}

template <typename Tp, std::ranges::output_range<Tp> RangeTp>
    requires(!std::ranges::forward_range<RangeTp>)
constexpr TakeView<std::ranges::views::all_t<RangeTp>>::TakeView(
    RangeTp base, difference_type count)
    : range_{std::move(base)}, limit_{count} {}

template <typename Tp, std::ranges::output_range<Tp> RangeTp>
    requires(!std::ranges::forward_range<RangeTp>)
[[nodiscard]] constexpr auto TakeView<std::ranges::views::all_t<RangeTp>>::base(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.range_);
}

template <typename Tp, std::ranges::output_range<Tp> RangeTp>
    requires(!std::ranges::forward_range<RangeTp>)
[[nodiscard]] constexpr TakeView<std::ranges::views::all_t<RangeTp>>::iterator
TakeView<std::ranges::views::all_t<RangeTp>>::begin() {
    return iterator{std::ranges::begin(range_)};
}

template <typename Tp, std::ranges::output_range<Tp> RangeTp>
    requires(!std::ranges::forward_range<RangeTp>)
[[nodiscard]] constexpr TakeView<std::ranges::views::all_t<RangeTp>>::sentinel
TakeView<std::ranges::views::all_t<RangeTp>>::end() {
    return sentinel{std::ranges::end(range_), limit_};
}

}  // namespace koda::ranges

namespace koda::views {

template <std::ranges::viewable_range RangeTp>
[[nodiscard]] constexpr auto TakeViewAdaptorClosure::operator()(
    RangeTp&& range) const {
    return ranges::TakeView{std::forward<RangeTp>(range), limit};
}

[[nodiscard]] constexpr TakeViewAdaptorClosure TakeViewAdaptor::operator()(
    std::ptrdiff_t limit) const {
    return TakeViewAdaptorClosure{limit};
}

}  // namespace koda::views
