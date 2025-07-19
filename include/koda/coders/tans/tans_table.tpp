#pragma once

#include <koda/utils/utils.hpp>

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
    number_of_states_ = normalize_to.value_or(total_size);
    ValidateNumberOfStates();
    ValidateStepSize(step);

    CountTp state = init_state % number_of_states_;

    state_table_.resize(number_of_states_);

    CountTp occ_accumulator = 0;
    CountTp norm_accumulator = 0;
    for (const auto& [token, occurences] : count) {
        const CountTp limit =
            static_cast<double>(occ_accumulator + occurences) *
                number_of_states_ / total_size -
            norm_accumulator;
        for (CountTp i = 0; i < limit; ++i) {
            state_table_[state] = token;
            state = (state + step) % number_of_states_;
        }
        states_per_token_.Emplace(token, limit);
        occ_accumulator += occurences;
        norm_accumulator += limit;
    }
}

template <typename Token, std::integral CountTp>
[[nodiscard]] constexpr const std::vector<Token>&
TansInitTable<Token, CountTp>::state_table() const noexcept {
    return state_table_;
}

template <typename Token, std::integral CountTp>
[[nodiscard]] constexpr const Map<Token, CountTp>&
TansInitTable<Token, CountTp>::states_per_token() const noexcept {
    return states_per_token_;
}

template <typename Token, std::integral CountTp>
[[nodiscard]] constexpr CountTp
TansInitTable<Token, CountTp>::number_of_states() const noexcept {
    return number_of_states_;
}

template <typename Token, std::integral CountTp>
constexpr void TansInitTable<Token, CountTp>::ValidateStepSize(
    CountTp step) const {
    if (step == 1) {
        return;
    }
    if (!step || step >= number_of_states_) [[unlikely]] {
        throw std::logic_error{
            std::format("Invalid step size, got: {}, expected 1 <= step < {}",
                        step, number_of_states_)};
    }
    if (number_of_states_ % step == 0) [[unlikely]] {
        throw std::logic_error{std::format(
            "Step size ({}) cannot be a multiple of the number of states ({})",
            step, number_of_states_)};
    }
}

template <typename Token, std::integral CountTp>
constexpr void TansInitTable<Token, CountTp>::ValidateNumberOfStates() const {
    if (!IsPowerOf2(number_of_states_)) [[unlikely]] {
        throw std::logic_error{
            std::format("Number of states must be a power of two, got ({})",
                        number_of_states_)};
    }
}

}  // namespace koda
