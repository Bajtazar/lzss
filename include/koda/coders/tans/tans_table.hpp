#pragma once

#include <koda/collections/map.hpp>

#include <cinttypes>
#include <cstddef>
#include <optional>
#include <vector>

namespace koda {

template <typename Token, std::integral CountTp>
struct TansInitTable {
    explicit constexpr TansInitTable(
        const Map<Token, CountTp>& count, size_t init_state = 0,
        const size_t step = 1,
        std::optional<size_t> normalize_to = std::nullopt);

    std::vector<Token> symbols;
    Map<Token, CountTp> counts;
    size_t state_sentinel;
};

}  // namespace koda

#include <koda/coders/tans/tans_table.tpp>
