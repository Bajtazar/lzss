#pragma once

namespace koda {

namespace ranges {

template <typename Range>
    requires std::ranges::output_range<Range, std::ranges::range_value_t<Range>>
template <ViewableDistinctRange<InsertFromBackView> RangeFwdTp>
constexpr InsertFromBackView<Range>::InsertFromBackView(RangeFwdTp&& range)
    : range_{std::forward<RangeFwdTp>(range)} {}

template <typename Range>
    requires std::ranges::output_range<Range, std::ranges::range_value_t<Range>>
[[nodiscard]] constexpr BackInserterIterator<Range>
InsertFromBackView<Range>::begin() const {
    return BackInserterIterator{range_.get()};
}

template <typename Range>
    requires std::ranges::output_range<Range, std::ranges::range_value_t<Range>>
[[nodiscard]] /*static*/ consteval std::default_sentinel_t
InsertFromBackView<Range>::end() noexcept {
    return std::default_sentinel;
}

template <typename Range>
    requires std::ranges::output_range<Range, std::ranges::range_value_t<Range>>
[[nodiscard]] constexpr auto&& InsertFromBackView<Range>::range(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.range_);
}

}  // namespace ranges

namespace views {

template <std::ranges::viewable_range Range>
[[nodiscard]] constexpr auto InsertFromBackAdaptorClosure::operator()(
    Range&& range) const {
    return ranges::InsertFromBackView{std::forward<Range>(range)};
}

}  // namespace views

}  // namespace koda
