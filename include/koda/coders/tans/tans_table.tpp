#pragma once

#include <algorithm>
#include <bit>
#include <format>
#include <ranges>
#include <stdexcept>

namespace koda {

template <typename Token, std::integral CountTp>
constexpr TansInitTable<Token, CountTp>::TansInitTable(
    const Map<Token, CountTp>& count, CountTp init_state, const CountTp step,
    std::optional<CountTp> normalize_to) {
    const CountTp total_size =
        std::ranges::fold_left(count | std::views::values, 0, std::plus<>{});
    state_sentinel_ = normalize_to.value_or(total_size);
    ValidateSentinelSize();
    ValidateStepSize(step);

    CountTp state = init_state % state_sentinel_;

    symbols_.resize(state_sentinel_);

    CountTp occ_accumulator = 0;
    CountTp norm_accumulator = 0;
    for (const auto& [token, occurences] : count) {
        const CountTp limit =
            static_cast<double>(occ_accumulator + occurences) *
                state_sentinel_ / total_size -
            norm_accumulator;
        for (CountTp i = 0; i < limit; ++i) {
            symbols_[state] = token;
            state = (state + step) % state_sentinel_;
        }
        counts_.Emplace(token, limit);
        occ_accumulator += occurences;
        norm_accumulator += limit;
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
            std::format("Invalid step size, got: {}, expected 1 <= step < {}",
                        step, state_sentinel_)};
    }
    if (state_sentinel_ % step == 0) [[unlikely]] {
        throw std::logic_error{
            std::format("Step size ({}) cannot be a multiple of the state sentinel ({})",
                        step, state_sentinel_)};
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
