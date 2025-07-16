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
        const TansInitTable<Token, Count>& init_table)
        : decoding_table_{BuildDecodingTable(init_table)},
          receiver_{BitIter{std::ranges::begin(received_bits_)},
                    std::next(BitIter{std::ranges::begin(received_bits_)},
                              IntFloorLog2(decoding_table_.size()))} {}

    constexpr auto Initialize(BitInputRange auto&& input) {
        return std::forward<decltype(input)>(input);
    }

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output) {
        auto in_iter = std::ranges::begin(input);
        auto in_sent = std::ranges::end(input);

        auto out_iter = std::ranges::begin(output);
        auto out_sent = std::ranges::end(output);

        for (; (in_iter != in_sent) && (out_iter != out_sent); ++out_iter) {
            in_iter = SetReceiver(in_iter, in_sent);
            *out_iter = DecodeToken();
        }

        return CoderResult{std::move(in_iter), std::move(in_iter),
                           std::move(out_iter), std::move(out_sent)};
    }

   private:
    using BitIter = BigEndianOutputBitIter<const State*>;
    using BitRange = std::pair<BitIter, BitIter>;
    using State = Count;

    struct DecodingEntry {
        Token symbol;
        State next_state;
        uint8_t bit_count;
    };

    std::vector<DecodingEntry> decoding_table_;
    BitRange receiver_;
    State state_;
    State received_bits_[1];

    constexpr auto SetReceiver(auto iter, const auto& sent) {
        auto& state_iter = receiver_.first;
        const auto& state_sent = receiver_.second;

        for (; (iter != sent) && (state_iter != state_sent); ++iter;
             ++state_iter) {
            *state_iter = *iter;
        }

        return iter;
    }

    constexpr Token DecodeToken() {
        auto shift = CHAR_BIT * sizeof(State) - state_sent.Position();
        state_ += received_bits_[0] >> shift;

        const auto& decoding_entry = decoding_table_[state_];
        Token token = decoding_entry.symbol;
        state_ = decoding_entry.next_state;

        receiver_ = {BitIter{std::ranges::begin(received_bits_)},
                     std::next(BitIter{std::ranges::begin(received_bits_)},
                               decoding_entry.bit_count)};
    }

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
