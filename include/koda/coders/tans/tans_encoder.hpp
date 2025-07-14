#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/collections/map.hpp>
#include <koda/utils/utils.hpp>

#include <cassert>
#include <cmath>

namespace koda {

template <typename Token, typename Count>
class TansEncoder : public EncoderInterface<Token, TansEncoder<Token, Count>> {
   public:
    constexpr explicit TansEncoder(
        const TansInitTable<Token, Count>& init_table)
        : offset_map_{BuildStartOffsetMap(init_table)},
          number_of_bits_{BuildNumberOfBitsMap(init_table)},
          encoding_table_{BuildEncodingTable(init_table, offset_map_)} {}

   private:
    Map<Token, Count> offset_map_;
    Map<Count, uint8_t> number_of_bits_;
    std::vector<Count> encoding_table_;

    static constexpr Map<Token, Count> BuildSaturationMap(
        const TansInitTable<Token, Count>& init_table) {
        Map<Token, Count> saturation_map;

        for (const auto& [token, count] : init_table.counts()) {
            saturation_map.Emplace(
                token,
                static_cast<Count>(std::log2(
                    static_cast<double>(init_table.state_sentinel()) / count)));
        }

        return saturation_map;
    }

    static constexpr Map<Count, uint8_t> BuildNumberOfBitsMap(
        const TansInitTable<Token, Count>& init_table) {
        auto saturation_map = BuildSaturationMap(init_table);

        return {BuildSaturationMap(init_table) |
                std::views::transform([&](const auto& saturation_tuple) {
                    const auto& [token, saturation] = saturation_tuple;

                    auto count_iter = init_table.counts().Find(token);
                    assert(count_iter != init_table.end());

                    return {token,
                            2 * init_table.state_sentinel() * saturation -
                                count_iter->second * (1uz << saturation)};
                })};
    }

    static constexpr Map<Token, Count> BuildStartOffsetMap(
        const TansInitTable<Token, Count>& init_table) {
        Map<Token, Count> offset_map;

        Count accumulator = init_table.state_sentinel();
        for (const auto& [token, count] : init_table.counts()) {
            offset_map.Emplace(token, accumulator);
            accumulator -= count;
        }

        return offset_map;
    }

    static constexpr std::vector<Count> BuildEncodingTable(
        const TansInitTable<Token, Count>& init_table,
        const Map<Token, Count>& start_offset_map) {
        const auto sentinel = init_table.state_sentinel();
        std::vector<Count> encoding_table(sentinel);
        Map<Token, Count> next = init_table.counts();

        for (Count i = sentinel; i < 2 * sentinel; ++i) {
            const auto& token = init_table.symbols()[i - sentinel];
            auto offset_iter = start_offset_map.Find(token);
            auto next_iter = next.Find(token);
            assert(offset_iter != start_offset_map.end());
            assert(next_iter != next.end());

            encoding_table[(next->second)++ - offset_iter->second] = i;
        }

        return encoding_table;
    }
};

}  // namespace koda
