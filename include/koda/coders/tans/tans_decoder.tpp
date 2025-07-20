#pragma once

#include <koda/utils/utils.hpp>

#include <cassert>

namespace koda {

template <typename Token, typename Count, typename State>
constexpr TansDecoder<Token, Count, State>::TansDecoder(
    const TansInitTable<Token, Count>& init_table)
    : decoding_table_{BuildDecodingTable(init_table)},
      receiver_{BitIter{std::ranges::begin(received_bits_)},
                BitIter{std::ranges::begin(received_bits_),
                        IntFloorLog2(decoding_table_.size())}} {}

template <typename Token, typename Count, typename State>
constexpr auto TansDecoder<Token, Count, State>::Initialize(
    BitInputRange auto&& input) {
    return std::forward<decltype(input)>(input);
}

template <typename Token, typename Count, typename State>
constexpr auto TansDecoder<Token, Count, State>::Decode(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    if (decoding_table_.size() == 1) {
        return HandleDiracDelta(std::forward<decltype(input)>(input),
                                std::forward<decltype(output)>(output));
    }

    auto in_iter = std::ranges::begin(input);
    auto in_sent = std::ranges::end(input);

    auto out_iter = std::ranges::begin(output);
    auto out_sent = std::ranges::end(output);

    while ((in_iter != in_sent) && (out_iter != out_sent)) {
        in_iter = SetReceiver(in_iter, in_sent);
        if (receiver_.first == receiver_.second) {
            *out_iter++ = DecodeToken();
        }
    }

    return CoderResult{std::move(in_iter), std::move(in_sent),
                       std::move(out_iter), std::move(out_sent)};
}

template <typename Token, typename Count, typename State>
constexpr auto TansDecoder<Token, Count, State>::HandleDiracDelta(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    auto out_iter = std::ranges::begin(output);
    auto out_sent = std::ranges::end(output);
    const Token& value = decoding_table_.front().symbol;
    auto out = std::ranges::fill(std::move(out_iter), out_sent, value);
    return CoderResult{std::forward<decltype(input)>(input), std::move(out),
                       std::move(out_sent)};
}

template <typename Token, typename Count, typename State>
constexpr auto TansDecoder<Token, Count, State>::SetReceiver(auto iter,
                                                             const auto& sent) {
    auto& state_iter = receiver_.first;
    const auto& state_sent = receiver_.second;

    for (; (iter != sent) && (state_iter != state_sent); ++iter, ++state_iter) {
        *state_iter = *iter;
    }

    return iter;
}

template <typename Token, typename Count, typename State>
constexpr Token TansDecoder<Token, Count, State>::DecodeToken() {
    receiver_.first.Flush();
    state_ += received_bits_[0] >> receiver_.second.Position();

    const auto& decoding_entry = decoding_table_[state_];
    Token token = decoding_entry.symbol;
    state_ = decoding_entry.next_state;

    receiver_ = {
        BitIter{std::ranges::begin(received_bits_)},
        BitIter{std::ranges::begin(received_bits_), decoding_entry.bit_count}};
    return token;
}

template <typename Token, typename Count, typename State>
/*static*/ constexpr std::vector<
    typename TansDecoder<Token, Count, State>::DecodingEntry>
TansDecoder<Token, Count, State>::BuildDecodingTable(
    const TansInitTable<Token, Count>& init_table) {
    const auto number_of_states = init_table.number_of_states();
    std::vector<DecodingEntry> decoding_table;
    decoding_table.reserve(number_of_states);
    Map<Token, Count> next = init_table.states_per_token();

    for (const auto& token : init_table.state_table()) {
        auto state = next.At(token)++;
        uint8_t bit_count =
            IntFloorLog2(number_of_states) - IntFloorLog2(state);
        auto new_state = (state << bit_count) - number_of_states;
        decoding_table.emplace_back(token, new_state, bit_count);
    }
    return decoding_table;
}

}  // namespace koda
