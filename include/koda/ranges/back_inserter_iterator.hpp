#pragma once

#include <koda/utils/concepts.hpp>

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
    template <ViewableDistinctRange<InsertFromBackView> RangeFwdTp>
    constexpr InsertFromBackView(RangeFwdTp&& range);

    [[nodiscard]] constexpr BackInserterIterator<Range> begin() const;

    [[nodiscard]] static consteval std::default_sentinel_t end() noexcept;

    [[nodiscard]] constexpr auto&& range(this auto&& self);

   private:
    std::reference_wrapper<Range> range_;
};

template <ViewableDistinctRange<InsertFromBackView> Range>
InsertFromBackView(Range&& range)
    -> InsertFromBackView<std::remove_cvref_t<Range>>;

}  // namespace ranges

namespace views {

struct InsertFromBackAdaptorClosure
    : public std::ranges::range_adaptor_closure<InsertFromBackAdaptorClosure> {
    template <std::ranges::viewable_range Range>
    [[nodiscard]] constexpr auto operator()(Range&& range) const;
};

inline constexpr InsertFromBackAdaptorClosure InsertFromBack{};

}  // namespace views

}  // namespace koda

#include <koda/ranges/back_inserter_iterator.tpp>
