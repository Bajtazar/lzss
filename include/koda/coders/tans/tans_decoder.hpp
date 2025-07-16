#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/collections/map.hpp>
#include <koda/utils/utils.hpp>

#include <cassert>
#include <cmath>

namespace koda {

template <typename Token, typename Count>
class TansDecoder : public DecoderInterface<Token, TansDecoder<Token, Count>> {
   public:
    constexpr explicit TansDecoder(
        const TansInitTable<Token, Count>& init_table);

   private:
    using State = Count;

    struct DecodingEntry {
        Token symbol;
        State next_state_;
        uint8_t bit_count;
    };

    std::vector<DecodingEntry> decoding_table_;
    State state_;

    static constexpr std::vector<DecodingEntry> BuildDecodingTable(
        const TansInitTable<Token, Count>& init_table) {
        const auto number_of_states = init_table.number_of_states();
        std::vector<DecodingEntry> decoding_table;
        Map<Token, Count> next = init_table.states_per_token();

        for (Count i = 0; i < number_of_states; ++i) {
            const auto& token = init_table.state_table()[i];
            auto state = next.At(token)++;
            uint8_t bit_count = std::ceil(std::log2(
                static_cast<double>(init_table.number_of_states()) / count));
            auto new_state = (state << bit_count) - number_of_states;
            decoding_table.emplace_back(token, new_state, bit_count);
        }
        return decoding_table;
    }
};

}  // namespace koda
