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
          renorm_map_{BuildRenormalizationOffsetMap(init_table)},
          encoding_table_{BuildEncodingTable(init_table, offset_map_)},
          state_{init_table.number_of_states()} {}

    auto Flush(BitOutputRange auto&& output) {
        auto sentinel = std::ranges::end(output);
        auto iter = FlushEmitter(std::ranges::begin(output), sentinel);

        if (iter == sentinel || encoding_table_.empty()) {
            return std::ranges::subrange{std::move(iter), std::move(sentinel)};
        }

        emitted_bits_[0] = state_;
        BitIter start_iter{std::ranges::begin(emitted_bits_)};
        emitter_ = std::pair{
            start_iter,
            std::next(start_iter, IntFloorLog2(encoding_table_.size()))};
        encoding_table_.clear();

        return FlushEmitter(std::move(iter), sentinel);
    }

   private:
    using BitIter = LittleEndianInputBitIter<const Count*>;
    using BitRange = std::pair<BitIter, BitIter>;
    using SCount = std::make_signed_t<Count>;

    Map<Token, SCount> offset_map_;
    Map<Token, uint8_t> renorm_map_;
    std::vector<Count> encoding_table_;
    BitRange emitter_;
    Count state_;
    Count emitted_bits_[1];

    constexpr void SetEmitter(auto& input_iter) {
        auto token = *input_iter++;
        auto bit_count =
            (state_ + renorm_map_.At(token)) / (2 * encoding_table_.size());
        assert(bit_count <= CHAR_BIT * sizeof(Count));
        emitted_bits_[0] = state_;
        state_ = encoding_table_[offset_map_.At(token) + (state_ >> bit_count)];
        BitIter start_iter{std::ranges::begin(emitted_bits_)};
        emitter_ = std::pair{start_iter, std::next(start_iter, bit_count)};
    }

    constexpr auto FlushEmitter(auto output_iter, const auto& output_sent) {
        auto& bit_iter = emitter_.first;
        const auto& bit_sent = emitter_.second;

        for (; (bit_iter != bit_sent) && (output_iter != output_sent);
             ++bit_iter, ++bit_sent) {
            *output_iter = *bit_iter;
        }
        return output_iter;
    }

    static constexpr Map<Token, Count> BuildSaturationMap(
        const TansInitTable<Token, Count>& init_table) {
        Map<Token, Count> saturation_map;

        for (const auto& [token, count] : init_table.states_per_token()) {
            saturation_map.Emplace(
                token, static_cast<Count>(std::log2(
                           static_cast<double>(init_table.number_of_states()) /
                           count)));
        }

        return saturation_map;
    }

    static constexpr Map<Token, uint8_t> BuildRenormalizationOffsetMap(
        const TansInitTable<Token, Count>& init_table) {
        auto saturation_map = BuildSaturationMap(init_table);

        return Map<Token, uint8_t>{
            BuildSaturationMap(init_table) |
            std::views::transform([&](const auto& saturation_tuple) {
                const auto& [token, saturation] = saturation_tuple;

                return std::pair{
                    token, 2 * init_table.number_of_states() * saturation -
                                   init_table.states_per_token().At(token)
                               << saturation};
            })};
    }

    static constexpr Map<Token, SCount> BuildStartOffsetMap(
        const TansInitTable<Token, Count>& init_table) {
        Map<Token, SCount> offset_map;

        Count accumulator = 0;
        for (const auto& [token, count] : init_table.states_per_token()) {
            offset_map.Emplace(token, static_cast<SCount>(accumulator) - count);
            accumulator += count;
        }

        return offset_map;
    }

    static constexpr std::vector<Count> BuildEncodingTable(
        const TansInitTable<Token, Count>& init_table,
        const Map<Token, SCount>& start_offset_map) {
        const auto number_of_states = init_table.number_of_states();
        std::vector<Count> encoding_table(number_of_states);
        Map<Token, Count> next = init_table.states_per_token();

        for (Count i = 0; i < number_of_states; ++i) {
            const auto& token = init_table.state_table()[i];

            encoding_table[next.At(token)++ + start_offset_map.At(token)] =
                i + number_of_states;
        }

        return encoding_table;
    }
};

}  // namespace koda
