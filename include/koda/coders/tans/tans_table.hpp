#pragma once

#include <koda/collections/map.hpp>

#include <cinttypes>
#include <cstddef>
#include <optional>
#include <vector>

namespace koda {

template <typename Token, std::integral CountTp>
class TansInitTable {
   public:
    explicit constexpr TansInitTable(
        const Map<Token, CountTp>& count, CountTp init_state = 0,
        const CountTp step = 1,
        std::optional<CountTp> normalize_to = std::nullopt);

    [[nodiscard]] constexpr const std::vector<Token>& symbols() const noexcept;

    [[nodiscard]] constexpr const Map<Token, CountTp>& counts() const noexcept;

    [[nodiscard]] constexpr CountTp state_sentinel() const noexcept;

   private:
    std::vector<Token> symbols_;
    Map<Token, CountTp> counts_;
    CountTp state_sentinel_;

    constexpr void ValidateStepSize(CountTp step) const;

    constexpr void ValidateSentinelSize() const;
};

}  // namespace koda

#include <koda/coders/tans/tans_table.tpp>
