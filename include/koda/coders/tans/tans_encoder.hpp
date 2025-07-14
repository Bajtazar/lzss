#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/collections/map.hpp>
#include <koda/utils/utils.hpp>

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
};

}  // namespace koda
