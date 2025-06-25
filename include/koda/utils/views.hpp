#pragma once

#include <cstdlib>
#include <iterator>
#include <ranges>

namespace koda {

namespace ranges {

template <typename CountTp, typename IterTp,
          std::sentinel_for<IterTp> SentinelTp>
class OutputTakeSentinel {
   public:
    constexpr explicit OutputTakeSentinel(
        SentinelTp sentinel,
        CountTp limit =
            {}) noexcept(std::is_nothrow_move_constructible_v<IterTp>);

    constexpr explicit OutputTakeSentinel() noexcept(
        std::is_nothrow_move_constructible_v<IterTp>)
        requires std::is_default_constructible_v<SentinelTp>;

    [[nodiscard]] constexpr auto&& base(this auto&& self);

    [[nodiscard]] constexpr CountTp counter() const noexcept;

   private:
    SentinelTp sentinel_ = {};
    CountTp counter_ = 0;
};

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp>
class OutputTakeIterator {
   public:
    using value_type = Tp;
    using difference_type = std::ptrdiff_t;

    constexpr explicit OutputTakeIterator(
        IterTp iterator,
        CountTp counter =
            {}) noexcept(std::is_nothrow_move_constructible_v<IterTp>);

    constexpr explicit OutputTakeIterator() noexcept(
        std::is_nothrow_move_constructible_v<IterTp>)
        requires std::is_default_constructible_v<IterTp>;

    constexpr OutputTakeIterator& operator=(value_type value);

    [[nodiscard]] constexpr OutputTakeIterator& operator*() noexcept;

    constexpr OutputTakeIterator& operator++() noexcept;

    [[nodiscard]] constexpr OutputTakeIterator operator++(int) noexcept;

    [[nodiscard]] constexpr auto&& base(this auto&& self);

    [[nodiscard]] constexpr CountTp counter() const noexcept;

   private:
    IterTp iterator_;
    CountTp counter_;
};

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp,
          std::sentinel_for<IterTp> SentTp>
[[nodiscard]] constexpr bool operator==(
    const OutputTakeIterator<CountTp, Tp, IterTp>& left,
    const OutputTakeSentinel<CountTp, IterTp, SentTp>& right) noexcept;

template <typename CountTp, typename Tp, std::output_iterator<Tp> IterTp,
          std::sentinel_for<IterTp> SentTp>
[[nodiscard]] constexpr bool operator==(
    const OutputTakeSentinel<CountTp, IterTp, SentTp>& left,
    const OutputTakeIterator<CountTp, Tp, IterTp>& right) noexcept;

template <std::ranges::range RangeTp>
class TakeView : public std::ranges::take_view<RangeTp> {
   public:
    using std::ranges::take_view<RangeTp>::take_view;
};

template <std::ranges::range RangeTp>
    requires(std::ranges::output_range<RangeTp,
                                       std::ranges::range_value_t<RangeTp>> &&
             !std::ranges::forward_range<RangeTp>)
class TakeView<RangeTp> {
   public:
    using difference_type = std::ranges::range_difference_t<RangeTp>;
    using iterator =
        OutputTakeIterator<difference_type, std::ranges::range_value_t<RangeTp>,
                           std::ranges::iterator_t<RangeTp>>;
    using sentinel =
        OutputTakeSentinel<difference_type, std::ranges::iterator_t<RangeTp>,
                           std::ranges::sentinel_t<RangeTp>>;

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

}  // namespace ranges

namespace views {

struct TakeViewAdaptorClosure
    : public std::ranges::range_adaptor_closure<TakeViewAdaptorClosure> {
    constexpr explicit TakeViewAdaptorClosure(std::ptrdiff_t limit) noexcept;

    std::ptrdiff_t limit;

    template <std::ranges::viewable_range RangeTp>
    [[nodiscard]] constexpr auto operator()(RangeTp&& range) const;
};

struct TakeViewAdaptor {
    [[nodiscard]] constexpr TakeViewAdaptorClosure operator()(
        std::ptrdiff_t limit) const;
};

inline constexpr TakeViewAdaptor Take{};

}  // namespace views

}  // namespace koda

#include <koda/utils/views.tpp>
