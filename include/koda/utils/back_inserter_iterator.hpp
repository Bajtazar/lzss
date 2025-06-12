#pragma once

#include <iterator>
#include <ranges>

namespace koda {

template <typename ContainterTp>
struct BackInserterIterator : public std::back_insert_iterator<ContainterTp> {
    using difference_type = ptrdiff_t;
    using value_type = typename ContainterTp::value_type;

    using std::back_insert_iterator<ContainterTp>::back_insert_iterator;

    [[nodiscard]] friend constexpr bool operator==(
        [[maybe_unused]] BackInserterIterator const& left,
        [[maybe_unused]] std::default_sentinel_t sentinel) noexcept {
        return false;
    }

    [[nodiscard]] friend constexpr bool operator==(
        [[maybe_unused]] std::default_sentinel_t sentinel,
        [[maybe_unused]] BackInserterIterator const& right) noexcept {
        return false;
    }

    constexpr BackInserterIterator& operator++() { return *this; }

    constexpr BackInserterIterator operator++(int) { return *this; }
};

namespace ranges {

template <typename Range>
    requires std::ranges::output_range<Range, std::ranges::range_value_t<Range>>
class InsertFromBackView
    : public std::ranges::view_interface<InsertFromBackView<Range>> {
   public:
    template <std::ranges::viewable_range RangeFwdTp>
    constexpr InsertFromBackView(RangeFwdTp&& range)
        : range_{std::forward<RangeFwdTp>(range)} {}

    [[nodiscard]] constexpr BackInserterIterator<Range> begin() const {
        return BackInserterIterator{range_.get()};
    }

    [[nodiscard]] static consteval std::default_sentinel_t end() noexcept {
        return std::default_sentinel;
    }

    [[nodiscard]] constexpr auto&& range(this auto&& self) {
        return std::forward_like<decltype(self)>(self.range_);
    }

   private:
    std::reference_wrapper<Range> range_;
};

template <std::ranges::viewable_range Range>
InsertFromBackView(Range&& range)
    -> InsertFromBackView<std::remove_cvref_t<Range>>;

}  // namespace ranges

namespace views {

struct InsertFromBackAdaptorClosure
    : public std::ranges::range_adaptor_closure<InsertFromBackAdaptorClosure> {
    template <std::ranges::viewable_range Range>
    [[nodiscard]] constexpr auto operator()(Range&& range) const {
        return ranges::InsertFromBackView{std::forward<Range>(range)};
    }
};

inline constexpr InsertFromBackAdaptorClosure InsertFromBack{};

}  // namespace views

}  // namespace koda
