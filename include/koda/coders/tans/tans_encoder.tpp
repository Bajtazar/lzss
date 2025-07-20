#pragma once

#include <koda/utils/utils.hpp>

#include <cassert>
#include <cmath>
#include <cstdlib>

namespace koda {

template <typename Token, typename Count, typename State>
constexpr TansEncoder<Token, Count, State>::TansEncoder(
    const TansInitTable<Token, Count>& init_table)
    : saturation_map_{BuildSaturationMap(init_table)},
      offset_map_{BuildStartOffsetMap(init_table)},
      renorm_map_{BuildRenormalizationOffsetMap(init_table, saturation_map_)},
      encoding_table_{BuildEncodingTable(init_table, offset_map_)},
      state_{init_table.number_of_states()},
      shift_{static_cast<uint8_t>(
          1 + IntFloorLog2(init_table.number_of_states()))} {}

template <typename Token, typename Count, typename State>
constexpr float TansEncoder<Token, Count, State>::TokenBitSize(
    Token token) const {
    return saturation_map_.At(token);
}

template <typename Token, typename Count, typename State>
constexpr auto TansEncoder<Token, Count, State>::Encode(
    InputRange<Token> auto&& input, BitOutputRange auto&& output) {
    auto sentinel = std::ranges::end(output);
    auto iter = FlushEmitter(std::ranges::begin(output), sentinel);

    if (iter == sentinel) {
        return CoderResult{std::forward<decltype(input)>(input),
                           std::move(iter), std::move(sentinel)};
    }

    return EncodeTokens(std::forward<decltype(input)>(input), std::move(iter),
                        std::move(sentinel));
}

template <typename Token, typename Count, typename State>
constexpr auto TansEncoder<Token, Count, State>::Flush(
    BitOutputRange auto&& output) {
    auto sentinel = std::ranges::end(output);
    auto iter = FlushEmitter(std::ranges::begin(output), sentinel);

    if (iter == sentinel || encoding_table_.empty()) {
        return std::ranges::subrange{std::move(iter), std::move(sentinel)};
    }

    // Encode final state with uniform distrib to simplify process
    state_ -= encoding_table_.size();
    SetEmitter(IntFloorLog2(encoding_table_.size()));

    return std::ranges::subrange{FlushEmitter(std::move(iter), sentinel),
                                 sentinel};
}

template <typename Token, typename Count, typename State>
constexpr auto TansEncoder<Token, Count, State>::EncodeTokens(
    InputRange<Token> auto&& input, auto iter, auto sentinel) {
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);

    for (; (iter != sentinel) && (input_iter != input_sent); ++input_iter) {
        EncodeToken(*input_iter);
        iter = FlushEmitter(iter, sentinel);
    }

    return CoderResult{std::move(input_iter), std::move(input_sent),
                       std::move(iter), std::move(sentinel)};
}

template <typename Token, typename Count, typename State>
constexpr void TansEncoder<Token, Count, State>::EncodeToken(
    const auto& token) {
    auto bit_count = (state_ + renorm_map_.At(token)) >> shift_;
    assert(bit_count <= CHAR_BIT * sizeof(Count));
    SetEmitter(bit_count);
    state_ = encoding_table_[offset_map_.At(token) + (state_ >> bit_count)];
}

template <typename Token, typename Count, typename State>
constexpr void TansEncoder<Token, Count, State>::SetEmitter(Count bit_count) {
    emitted_bits_[0] = state_;
    emitter_ = std::pair{BitIter{std::ranges::begin(emitted_bits_)},
                         BitIter{std::ranges::begin(emitted_bits_), bit_count}};
}

template <typename Token, typename Count, typename State>
constexpr auto TansEncoder<Token, Count, State>::FlushEmitter(
    auto output_iter, const auto& output_sent) {
    auto& bit_iter = emitter_.first;
    const auto& bit_sent = emitter_.second;

    for (; (bit_iter != bit_sent) && (output_iter != output_sent);
         ++output_iter, ++bit_iter) {
        *output_iter = *bit_iter;
    }
    return output_iter;
}

template <typename Token, typename Count, typename State>
/*static*/ constexpr Map<Token, uint8_t>
TansEncoder<Token, Count, State>::BuildSaturationMap(
    const TansInitTable<Token, Count>& init_table) {
    return Map<Token, uint8_t>{
        init_table.states_per_token() |
        std::views::transform(
            [max_bit_size = IntFloorLog2(init_table.number_of_states())](
                const auto& entry) {
                const auto& [token, count] = entry;
                return std::pair{token, max_bit_size - IntFloorLog2(count)};
            })};
}

template <typename Token, typename Count, typename State>
/*static*/ constexpr Map<Token, State>
TansEncoder<Token, Count, State>::BuildRenormalizationOffsetMap(
    const TansInitTable<Token, Count>& init_table,
    const Map<Token, uint8_t>& saturation_map) {
    return Map<Token, Count>{
        BuildSaturationMap(init_table) |
        std::views::transform(
            [&, max_bit_size = 1 + IntFloorLog2(init_table.number_of_states())](
                const auto& saturation_tuple) {
                const auto& [token, saturation] = saturation_tuple;
                auto max = saturation << max_bit_size;
                auto min = init_table.states_per_token().At(token)
                           << saturation;
                return std::pair{token, max > min ? (max - min) : 0};
            })};
}

template <typename Token, typename Count, typename State>
/*static*/ constexpr Map<Token,
                         typename TansEncoder<Token, Count, State>::SState>
TansEncoder<Token, Count, State>::BuildStartOffsetMap(
    const TansInitTable<Token, Count>& init_table) {
    return Map<Token, SState>{
        init_table.states_per_token() |
        std::views::transform([accumulator =
                                   Count{0}](const auto& entry) mutable {
            const auto& [token, count] = entry;
            SState offset =
                static_cast<SState>(accumulator) - static_cast<SState>(count);
            accumulator += count;
            return std::pair{token, offset};
        })};
}

template <typename Token, typename Count, typename State>
/*static*/ constexpr std::vector<State>
TansEncoder<Token, Count, State>::BuildEncodingTable(
    const TansInitTable<Token, Count>& init_table,
    const Map<Token, SState>& start_offset_map) {
    const auto number_of_states = init_table.number_of_states();
    std::vector<Count> encoding_table(number_of_states);
    Map<Token, Count> next = init_table.states_per_token();

    for (const auto& [index, token] :
         std::views::enumerate(init_table.state_table())) {
        encoding_table[next.At(token)++ + start_offset_map.At(token)] =
            index + number_of_states;
    }

    return encoding_table;
}

}  // namespace koda
