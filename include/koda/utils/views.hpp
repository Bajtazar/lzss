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

    constexpr explicit OutputTakeIterator(IterTp iterator) noexcept(
        std::is_nothrow_move_constructible_v<IterTp>);

    constexpr OutputTakeIterator& operator=(value_type value);

    [[nodiscard]] constexpr OutputTakeIterator& operator*(void) noexcept;

    constexpr OutputTakeIterator& operator++() noexcept;

    [[nodiscard]] constexpr OutputTakeIterator& operator++(int) noexcept;

    [[nodiscard]] constexpr auto&& base(this auto&& self);

    [[nodiscard]] constexpr size_t counter() const noexcept;

   private:
    IterTp iterator_;
    size_t counter_ = 0;
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

}  // namespace koda::ranges

#include <koda/utils/views.tpp>
