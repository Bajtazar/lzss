#pragma once

#include <cstdlib>
#include <iterator>
#include <ranges>

namespace koda::ranges {

template <typename IterTp, std::sentinel_for<IterTp> SentinelTp>
class OutputTakeSentinel {
   public:
    constexpr explicit OutputTakeSentinel(
        SentinelTp sentinel,
        size_t limit) noexcept(std::is_nothrow_move_constructible_v<IterTp>);

    [[nodiscard]] constexpr auto&& base(this auto&& self);

    [[nodiscard]] constexpr size_t counter() const noexcept;

   private:
    SentinelTp sentinel_;
    size_t counter_;
};

template <typename Tp, std::output_iterator<Tp> IterTp>
class OutputTakeIterator {
   public:
    using value_type = Tp;
    using difference_type = std::ptrdiff_t;

    constexpr explicit OutputTakeIterator(
        IterTp iterator,
        size_t counter =
            0) noexcept(std::is_nothrow_move_constructible_v<IterTp>);

    constexpr OutputTakeIterator& operator=(value_type value);

    [[nodiscard]] constexpr OutputTakeIterator& operator*() noexcept;

    constexpr OutputTakeIterator& operator++() noexcept;

    [[nodiscard]] constexpr OutputTakeIterator operator++(int) noexcept;

    [[nodiscard]] constexpr auto&& base(this auto&& self);

    [[nodiscard]] constexpr size_t counter() const noexcept;

   private:
    IterTp iterator_;
    size_t counter_;
};

template <typename Tp, std::output_iterator<Tp> IterTp,
          std::sentinel_for<IterTp> SentTp>
[[nodiscard]] constexpr bool operator==(
    const OutputTakeIterator<Tp, IterTp>& left,
    const OutputTakeSentinel<IterTp, SentTp>& right) noexcept;

template <typename Tp, std::output_iterator<Tp> IterTp,
          std::sentinel_for<IterTp> SentTp>
[[nodiscard]] constexpr bool operator==(
    const OutputTakeSentinel<IterTp, SentTp>& left,
    const OutputTakeIterator<Tp, IterTp>& right) noexcept;

template <std::ranges::range RangeTp>
class TakeView : public std::ranges::take_view<RangeTp> {
   public:
    using std::ranges::take_view<RangeTp>::take_view;
};

template <typename Tp, std::ranges::output_range<Tp> RangeTp>
    requires(!std::ranges::forward_range<RangeTp>)
class TakeView<std::ranges::views::all_t<RangeTp>> {
   public:
    using difference_type = std::ranges::range_difference_t<RangeTp>;
    using iterator = OutputTakeIterator<
        Tp, std::ranges::iterator_t<std::ranges::views::all_t<RangeTp>>>;
    using sentinel = OutputTakeSentinel<
        std::ranges::iterator_t<std::ranges::views::all_t<RangeTp>>,
        std::ranges::sentinel_t<std::ranges::views::all_t<RangeTp>>>;

    constexpr TakeView() noexcept
        requires std::default_initializable<RangeTp>
    = default;

    constexpr explicit TakeView(RangeTp base, difference_type count);

    [[nodiscard]] constexpr auto base(this auto&& self);

    [[nodiscard]] constexpr iterator begin();

    [[nodiscard]] constexpr sentinel end();

   private:
    RangeTp range_ = {};
    difference_type limit_ = 0;
};

template <typename RangeTp>
TakeView(RangeTp&&, std::ranges::range_difference_t<RangeTp>)
    -> TakeView<std::views::all_t<RangeTp>>;

}  // namespace koda::ranges

namespace koda::views {

struct TakeViewAdaptorClosure
    : public std::ranges::range_adaptor_closure<TakeViewAdaptorClosure> {
    std::ptrdiff_t limit;

    template <std::ranges::viewable_range RangeTp>
    [[nodiscard]] constexpr auto operator()(RangeTp&& range) const;
};

struct TakeViewAdaptor {
    [[nodiscard]] constexpr TakeViewAdaptorClosure operator()(
        std::ptrdiff_t limit) const;
};

inline constexpr TakeViewAdaptor Take{};

}  // namespace koda::views

#include <koda/utils/views.tpp>
