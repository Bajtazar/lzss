#include <koda/coders/huffman/huffman_decoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

#include "common.hpp"

static_assert(koda::Decoder<koda::HuffmanDecoder<uint8_t>, uint8_t>);

BeginConstexprTest(HuffmanDecoderTest, DecodeRandomDistribution) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    const koda::HuffmanTable<uint32_t> kTable = {
        HuffmanEntry{0, std::vector<bool>{0}},
        HuffmanEntry{1, std::vector<bool>{1, 0, 1, 0}},
        HuffmanEntry{5, std::vector<bool>{1, 1, 1}},
        HuffmanEntry{16, std::vector<bool>{1, 1, 0}},
        HuffmanEntry{32, std::vector<bool>{1, 0, 0}},
        HuffmanEntry{43, std::vector<bool>{1, 0, 1, 1}}};

    const std::vector<uint32_t> kExpected = {0, 1, 1, 16, 0, 32, 1, 43};

    std::vector<bool> stream = ConcatenateSymbols(kTable, kExpected);

    koda::HuffmanDecoder decoder{kTable};

    std::vector<uint32_t> decoded;

    decoder(stream, decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(decoded, kExpected);
}
EndConstexprTest;
