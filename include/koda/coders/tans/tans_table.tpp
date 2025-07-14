#pragma once

#include <algorithm>
#include <bit>
#include <ranges>

namespace koda {

template <typename Token, std::integral CountTp>
[[nodiscard]] TansInitTable<Token, CountTp>::TansInitTable(
    const Map<Token, CountTp>& count, CountTp init_state, const CountTp step,
    std::optional<CountTp> normalize_to) {
    const CountTp total_size =
        std::ranges::fold_left(count | std::views::values, 0, std::plus<>{});
    state_sentinel_ = normalize_to.value_or(total_size);
    ValidateSentinelSize();
    ValidateStepSize(step);

    CountTp state = init_state % state_sentinel_;

    symbols_.resize(state_sentinel_);

    for (const auto& [token, occurences] : count) {
        const CountTp limit =
            static_cast<double>(occurences) * state_sentinel_ / total_size;
        for (CountTp i = 0; i < limit; ++i) {
            symbols_[step] = token;
            state = (state + step) % state_sentinel_;
        }
        counts_.Emplace(token, limit);
    }
}

template <typename Token, std::integral CountTp>
[[nodiscard]] constexpr const std::vector<Token>&
TansInitTable<Token, CountTp>::symbols() const noexcept {
    return symbols_;
}

template <typename Token, std::integral CountTp>
[[nodiscard]] constexpr const Map<Token, CountTp>&
TansInitTable<Token, CountTp>::counts() const noexcept {
    return counts_;
}

template <typename Token, std::integral CountTp>
[[nodiscard]] constexpr CountTp TansInitTable<Token, CountTp>::state_sentinel()
    const noexcept {
    return state_sentinel_;
}

template <typename Token, std::integral CountTp>
constexpr void TansInitTable<Token, CountTp>::ValidateStepSize(
    CountTp step) const {
    if (!step || step >= state_sentinel_) [[unlikely]] {
        throw std::logic_error{
            std::format("Invalid step size, got: {}, expected 1 <= step < {}"),
            step, state_sentinel_};
    }
}

template <typename Token, std::integral CountTp>
constexpr void TansInitTable<Token, CountTp>::ValidateSentinelSize() const {
    // Maybe will work without it!
    // if (1uz << std::countr_zero(state_sentinel_) != state_sentinel_)
    //     [[unlikely]] {
    //     throw std::logic_error{std::format(
    //         "State sentinel is not a power of 2! (got {})",
    //         state_sentinel_)};
    // }
}

}  // namespace koda
