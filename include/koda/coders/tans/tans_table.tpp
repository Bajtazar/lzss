#pragma once

#include <koda/utils/formatted_exception.hpp>
#include <koda/utils/utils.hpp>

#include <algorithm>
#include <bit>
#include <ranges>

namespace koda {

template <typename Token, std::integral CountTp>
constexpr TansInitTable<Token, CountTp>::TansInitTable(
    const Map<Token, CountTp>& count, CountTp init_state, const CountTp step,
    std::optional<CountTp> normalize_to) {
    const CountTp total_size =
        std::ranges::fold_left(count | std::views::values, 0, std::plus<>{});
    const size_t number_of_states = normalize_to.value_or(total_size);
    ValidateNumberOfStates(number_of_states);
    ValidateStepSize(step, number_of_states);

    InitializeStateTable(count, init_state % number_of_states, step, total_size,
                         number_of_states);
}

template <typename Token, std::integral CountTp>
constexpr void TansInitTable<Token, CountTp>::InitializeStateTable(
    const Map<Token, CountTp>& count, CountTp state, CountTp step,
    CountTp total_size, size_t number_of_states) {
    state_table_.resize(number_of_states);

    std::vector<std::pair<Token, CountTp>> sorted_count{std::from_range, count};
    std::ranges::sort(sorted_count, std::less<>{},
                      &std::pair<Token, CountTp>::second);

    CountTp norm_accumulator = PopulateStateTable(sorted_count, state, step,
                                                  total_size, number_of_states);

    if (norm_accumulator != number_of_states) [[unlikely]] {
        throw FormattedException{
            "Invalid allocation of states, allocated ({}) slots but ({}) slots "
            "are available",
            norm_accumulator, number_of_states};
    }
}

template <typename Token, std::integral CountTp>
constexpr CountTp TansInitTable<Token, CountTp>::PopulateStateTable(
    const std::vector<std::pair<Token, CountTp>>& sorted_count, CountTp state,
    CountTp step, CountTp total_size, size_t number_of_states) {
    CountTp occ_accumulator = 0, norm_accumulator = 0;
    for (const auto& [token, occurences] : sorted_count) {
        const CountTp limit = std::max<CountTp>(
            static_cast<double>(occ_accumulator + occurences) *
                    number_of_states / total_size -
                norm_accumulator,
            1);
        for (CountTp i = 0; i < limit; ++i) {
            state_table_[state] = token;
            state = (state + step) % number_of_states;
        }
        states_per_token_.Emplace(token, limit);
        occ_accumulator += occurences;
        norm_accumulator += limit;
    }
    return norm_accumulator;
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
    return state_table_.size();
}

template <typename Token, std::integral CountTp>
constexpr void TansInitTable<Token, CountTp>::ValidateStepSize(
    CountTp step, size_t number_of_states) const {
    if (step == 1) {
        return;
    }
    if (!step || step >= number_of_states) [[unlikely]] {
        throw FormattedException{
            "Invalid step size, got: {}, expected 1 <= step < {}", step,
            number_of_states};
    }
    if (number_of_states % step == 0) [[unlikely]] {
        throw FormattedException{
            "Step size ({}) cannot be a multiple of the number of states ({})",
            step, number_of_states};
    }
}

template <typename Token, std::integral CountTp>
constexpr void TansInitTable<Token, CountTp>::ValidateNumberOfStates(
    size_t number_of_states) const {
    if (!IsPowerOf2(number_of_states)) [[unlikely]] {
        throw FormattedException{
            "Number of states must be a power of two, got ({})",
            number_of_states};
    }
}

}  // namespace koda
