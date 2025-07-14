#pragma once

#include <algorithm>
#include <ranges>

namespace koda {

template <typename Token, std::integral CountTp>
[[nodiscard]] TansInitTable<Token, CountTp>::TansInitTable(
    const Map<Token, CountTp>& count, size_t init_state = 0,
    const size_t step = 1, std::optional<size_t> normalize_to = std::nullopt) {
    const size_t total_size =
        std::ranges::fold_left(count | std::views::values, 0, std::plus<>{});
    state_sentinel = normalize_to.value_or(total_size);
    size_t state = init_state % state_sentinel;

    if (!step || step >= total_size) {
        throw std::runtime_error{
            std::format("Invalid step size, got: {}, expected 1 <= step < {}"),
            step, total_size};
    }

    symbols.resize(state_sentinel);

    for (const auto& [token, occurences] : count) {
        const size_t limit =
            static_cast<double>(occurences) * state_sentinel / total_size;
        for (size_t i = 0; i < limit; ++i) {
            table[step] = token;
            state = (state + step) % state_sentinel;
        }
        counts.Emplace(token, limit);
    }
}

}  // namespace koda
