#pragma once

#include <koda/coder.hpp>

#include <cstdlib>
#include <istream>
#include <ostream>

namespace koda {

template <typename Tp, typename IntermediateToken>
concept LzDistanceTable = requires(
    Tp table, DummyEncoder encoder, DummyDecoder<IntermediateToken> decoder,
    size_t distance, size_t match_length, IntermediateToken start_token,
    DummyBitInputRange input, DummyBitOutputRange output) {
    {
        table.EncodeDistance(output, distance, match_length, encoder)
    } -> std::same_as<void>;
    {
        table.DecodeDistance(input, start_token, decoder)
    } -> std::same_as<std::pair<size_t, size_t>>;
};

}  // namespace koda
