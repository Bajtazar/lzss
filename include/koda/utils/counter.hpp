#pragma once

#include <koda/collections/map.hpp>

namespace koda {

template <typename Token, std::integral CountTp = size_t>
class Counter {
   public:
    explicit constexpr Counter() noexcept = default;

    explicit constexpr Counter(std::ranges::input_range auto&& range);

    constexpr void Count(const Token& token);

    constexpr void CountRange(std::ranges::input_range auto&& range);

    [[nodiscard]] constexpr auto&& counted(this auto&& self) noexcept;

   private:
    Map<Token, Counter> counter_;
};

template <std::ranges::input_range Range>
Counter(Range&&) -> Counter<std::ranges::range_value_t<Range>>;

}  // namespace koda
