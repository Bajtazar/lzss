#pragma once

namespace koda {

template <typename Token, typename Count>
constexpr TansEncoder<Token, Count>::TansEncoder(
    const TansInitTable<Token, Count>& init_table)
    : saturation_map_{BuildSaturationMap(init_table)},
      offset_map_{BuildStartOffsetMap(init_table)},
      renorm_map_{BuildRenormalizationOffsetMap(init_table, saturation_map_)},
      encoding_table_{BuildEncodingTable(init_table, offset_map_)},
      state_{init_table.number_of_states()} {}

template <typename Token, typename Count>
constexpr float TansEncoder<Token, Count>::TokenBitSize(Token token) const {
    return saturation_map_.At(token);
}

template <typename Token, typename Count>
constexpr auto TansEncoder<Token, Count>::Encode(InputRange<Token> auto&& input,
                                                 BitOutputRange auto&& output) {
    auto sentinel = std::ranges::end(output);
    auto iter = FlushEmitter(std::ranges::begin(output), sentinel);

    if (iter == sentinel || encoding_table_.empty()) {
        return CoderResult{std::forward<decltype(input)>(input),
                           std::move(iter), std::move(sentinel)};
    }

    return EncodeTokens(std::forward<decltype(input)>(input), std::move(iter),
                        std::move(sentinel));
}

template <typename Token, typename Count>
constexpr auto TansEncoder<Token, Count>::Flush(BitOutputRange auto&& output) {
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

template <typename Token, typename Count>
constexpr auto TansEncoder<Token, Count>::EncodeTokens(
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

template <typename Token, typename Count>
constexpr void TansEncoder<Token, Count>::EncodeToken(const auto& token) {
    auto bit_count =
        (state_ + renorm_map_.At(token)) / (2 * encoding_table_.size());
    assert(bit_count <= CHAR_BIT * sizeof(Count));
    SetEmitter(bit_count);
    state_ = encoding_table_[offset_map_.At(token) + (state_ >> bit_count)];
}

template <typename Token, typename Count>
constexpr void TansEncoder<Token, Count>::SetEmitter(Count bit_count) {
    emitted_bits_[0] = state_;
    BitIter start_iter{std::ranges::begin(emitted_bits_)};
    emitter_ = std::pair{BitIter{std::ranges::begin(emitted_bits_)},
                         BitIter{std::ranges::begin(emitted_bits_), bit_count}};
}

template <typename Token, typename Count>
constexpr auto TansEncoder<Token, Count>::FlushEmitter(
    auto output_iter, const auto& output_sent) {
    auto& bit_iter = emitter_.first;
    const auto& bit_sent = emitter_.second;

    for (; (bit_iter != bit_sent) && (output_iter != output_sent);
         ++output_iter, ++bit_iter) {
        *output_iter = *bit_iter;
    }
    return output_iter;
}

template <typename Token, typename Count>
/*static*/ constexpr Map<Token, uint8_t>
TansEncoder<Token, Count>::BuildSaturationMap(
    const TansInitTable<Token, Count>& init_table) {
    Map<Token, uint8_t> saturation_map;

    for (const auto& [token, count] : init_table.states_per_token()) {
        saturation_map.Emplace(
            token,
            static_cast<uint8_t>(std::ceil(std::log2(
                static_cast<double>(init_table.number_of_states()) / count))));
    }

    return saturation_map;
}

template <typename Token, typename Count>
/*static*/ constexpr Map<Token, Count>
TansEncoder<Token, Count>::BuildRenormalizationOffsetMap(
    const TansInitTable<Token, Count>& init_table,
    const Map<Token, uint8_t>& saturation_map) {
    return Map<Token, Count>{
        BuildSaturationMap(init_table) |
        std::views::transform([&](const auto& saturation_tuple) {
            const auto& [token, saturation] = saturation_tuple;
            auto max = 2 * init_table.number_of_states() * saturation;
            auto min = init_table.states_per_token().At(token) << saturation;
            return std::pair{token, max > min ? (max - min) : 0};
        })};
}

template <typename Token, typename Count>
/*static*/ constexpr Map<Token, typename TansEncoder<Token, Count>::SState>
TansEncoder<Token, Count>::BuildStartOffsetMap(
    const TansInitTable<Token, Count>& init_table) {
    Map<Token, SState> offset_map;

    Count accumulator = 0;
    for (const auto& [token, count] : init_table.states_per_token()) {
        offset_map.Emplace(token, static_cast<SState>(accumulator) - count);
        accumulator += count;
    }

    return offset_map;
}

template <typename Token, typename Count>
/*static*/ constexpr std::vector<Count>
TansEncoder<Token, Count>::BuildEncodingTable(
    const TansInitTable<Token, Count>& init_table,
    const Map<Token, SState>& start_offset_map) {
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

}  // namespace koda
