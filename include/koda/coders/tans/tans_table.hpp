#pragma once

#include <koda/collections/map.hpp>

#include <algorithm>
#include <array>
#include <cinttypes>
#include <cstddef>
#include <optional>
#include <ranges>
#include <vector>

namespace koda {

template <typename Token>
using TansInitTable = std::vector<Token>;

template <typename Token, std::integral CountTp>
[[nodiscard]] TansInitTable<Token> MakeTansInitTable(
    const Map<Token, CountTp>& count, size_t init_state = 0,
    const size_t step = 1, std::optional<size_t> normalize_to = std::nullopt);

}  // namespace koda

#include <koda/coders/tans/tans_table.tpp>
