#pragma once

namespace koda {

namespace ranges {

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp,
          std::sentinel_for<IterTp> SentTp>
[[nodiscard]] constexpr bool operator==(
    const OutputTakeIterator<CountTp, Tp, IterTp>& left,
    const OutputTakeSentinel<CountTp, IterTp, SentTp>& right) noexcept {
    return (left.base() == right.base()) || (left.counter() == right.counter());
}

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp,
          std::sentinel_for<IterTp> SentTp>
[[nodiscard]] constexpr bool operator==(
    const OutputTakeSentinel<CountTp, IterTp, SentTp>& left,
    const OutputTakeIterator<CountTp, Tp, IterTp>& right) noexcept {
    return (left.base() == right.base()) || (left.counter() == right.counter());
}

template <typename CountTp, typename IterTp,
          std::sentinel_for<IterTp> SentinelTp>
constexpr OutputTakeSentinel<CountTp, IterTp, SentinelTp>::OutputTakeSentinel(
    SentinelTp sentinel,
    CountTp limit) noexcept(std::is_nothrow_move_constructible_v<IterTp>)
    : sentinel_{std::move(sentinel)}, counter_{limit} {}

template <typename CountTp, typename IterTp,
          std::sentinel_for<IterTp> SentinelTp>
constexpr OutputTakeSentinel<CountTp, IterTp, SentinelTp>::
    OutputTakeSentinel() noexcept(std::is_nothrow_move_constructible_v<IterTp>)
    requires std::is_default_constructible_v<SentinelTp>
{}

template <typename CountTp, typename IterTp,
          std::sentinel_for<IterTp> SentinelTp>
[[nodiscard]] constexpr auto&&
OutputTakeSentinel<CountTp, IterTp, SentinelTp>::base(this auto&& self) {
    return std::forward_like<decltype(self)>(self.sentinel_);
}

template <typename CountTp, typename IterTp,
          std::sentinel_for<IterTp> SentinelTp>
[[nodiscard]] constexpr CountTp
OutputTakeSentinel<CountTp, IterTp, SentinelTp>::counter() const noexcept {
    return counter_;
}

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp>
constexpr OutputTakeIterator<CountTp, Tp, IterTp>::OutputTakeIterator(
    IterTp iterator,
    CountTp counter) noexcept(std::is_nothrow_move_constructible_v<IterTp>)
    : iterator_{std::move(iterator)}, counter_{counter} {}

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp>
constexpr OutputTakeIterator<CountTp, Tp, IterTp>::
    OutputTakeIterator() noexcept(std::is_nothrow_move_constructible_v<IterTp>)
    requires std::is_default_constructible_v<IterTp>
{}

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp>
constexpr OutputTakeIterator<CountTp, Tp, IterTp>&
OutputTakeIterator<CountTp, Tp, IterTp>::operator=(value_type value) {
    *iterator_ = std::move(value);
    return *this;
}

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp>
[[nodiscard]] constexpr OutputTakeIterator<CountTp, Tp, IterTp>&
OutputTakeIterator<CountTp, Tp, IterTp>::operator*() noexcept {
    return *this;
}

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp>
constexpr OutputTakeIterator<CountTp, Tp, IterTp>&
OutputTakeIterator<CountTp, Tp, IterTp>::operator++() noexcept {
    ++iterator_;
    ++counter_;
    return *this;
}

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp>
[[nodiscard]] constexpr OutputTakeIterator<CountTp, Tp, IterTp>
OutputTakeIterator<CountTp, Tp, IterTp>::operator++(int) noexcept {
    return OutputTakeIterator{iterator_++, counter_++};
}

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp>
[[nodiscard]] constexpr auto&& OutputTakeIterator<CountTp, Tp, IterTp>::base(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.iterator_);
}

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp>
[[nodiscard]] constexpr CountTp
OutputTakeIterator<CountTp, Tp, IterTp>::counter() const noexcept {
    return counter_;
}

template <std::ranges::range RangeTp>
    requires(std::ranges::output_range<RangeTp,
                                       std::ranges::range_value_t<RangeTp>> &&
             !std::ranges::forward_range<RangeTp>)
constexpr TakeView<RangeTp>::TakeView(RangeTp base, difference_type count)
    : range_{std::move(base)}, limit_{count} {}

template <std::ranges::range RangeTp>
    requires(std::ranges::output_range<RangeTp,
                                       std::ranges::range_value_t<RangeTp>> &&
             !std::ranges::forward_range<RangeTp>)
[[nodiscard]] constexpr auto TakeView<RangeTp>::base(this auto&& self) {
    return std::forward_like<decltype(self)>(self.range_);
}

template <std::ranges::range RangeTp>
    requires(std::ranges::output_range<RangeTp,
                                       std::ranges::range_value_t<RangeTp>> &&
             !std::ranges::forward_range<RangeTp>)
[[nodiscard]] constexpr TakeView<RangeTp>::iterator TakeView<RangeTp>::begin() {
    return iterator{std::ranges::begin(range_)};
}

template <std::ranges::range RangeTp>
    requires(std::ranges::output_range<RangeTp,
                                       std::ranges::range_value_t<RangeTp>> &&
             !std::ranges::forward_range<RangeTp>)
[[nodiscard]] constexpr TakeView<RangeTp>::sentinel TakeView<RangeTp>::end() {
    return sentinel{std::ranges::end(range_), limit_};
}

}  // namespace ranges

namespace views {

constexpr TakeViewAdaptorClosure::TakeViewAdaptorClosure(
    std::ptrdiff_t limit) noexcept
    : limit{limit} {}

template <std::ranges::viewable_range RangeTp>
[[nodiscard]] constexpr auto TakeViewAdaptorClosure::operator()(
    RangeTp&& range) const {
    return ranges::TakeView{std::forward<RangeTp>(range), limit};
}

[[nodiscard]] constexpr TakeViewAdaptorClosure TakeViewAdaptor::operator()(
    std::ptrdiff_t limit) const {
    return TakeViewAdaptorClosure{limit};
}

}  // namespace views

}  // namespace koda
