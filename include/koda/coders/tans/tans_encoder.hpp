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
   private:
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

    static constexpr Map<Token, size_t> MakeStartOffsetMap(
        const TansInitTable<Token, Count>& init_table) {
        TansInitTable<Token, Count> offset_map;

        size_t accumulator = init_table.state_sentinel();
        for (const auto& [token, count] : init_table.counts()) {
            offset_map.Emplace(token, accumulator);
            accumulator -= count;
        }

        return offset_map;
    }
};

}  // namespace koda
