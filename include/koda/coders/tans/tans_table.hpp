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

    [[nodiscard]] constexpr const std::vector<Token>& state_table()
        const noexcept;

    [[nodiscard]] constexpr const Map<Token, CountTp>& states_per_token()
        const noexcept;

    [[nodiscard]] constexpr CountTp number_of_states() const noexcept;

   private:
    std::vector<Token> state_table_;
    Map<Token, CountTp> states_per_token_;

    constexpr void InitializeStateTable(const Map<Token, CountTp>& count,
                                        CountTp state, CountTp step,
                                        CountTp total_size,
                                        size_t number_of_states);

    constexpr CountTp PopulateStateTable(
        const std::vector<std::pair<Token, CountTp>>& sorted_count,
        CountTp state, CountTp step, CountTp total_size,
        size_t number_of_states);

    constexpr void ValidateStepSize(CountTp step,
                                    size_t number_of_states) const;

    constexpr void ValidateNumberOfStates(size_t number_of_states) const;
};

template <typename Token, std::integral CountTp, typename InitTp>
TansInitTable(const Map<Token, CountTp>&, InitTp&&)
    -> TansInitTable<Token, CountTp>;

template <typename Token, std::integral CountTp, typename InitTp,
          typename StepTp>
TansInitTable(const Map<Token, CountTp>&, InitTp&&, StepTp&&)
    -> TansInitTable<Token, CountTp>;

template <typename Token, std::integral CountTp, typename InitTp,
          typename StepTp, typename NormTp>
TansInitTable(const Map<Token, CountTp>&, InitTp&&, StepTp&&, NormTp&&)
    -> TansInitTable<Token, CountTp>;

}  // namespace koda

#include <koda/coders/tans/tans_table.tpp>
