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

BeginConstexprTest(HuffmanDecoderTest, DecodeGeometricDistribution) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    const koda::HuffmanTable<char> kTable = {
        HuffmanEntry{'t', std::vector<bool>{1}},
        HuffmanEntry{'r', std::vector<bool>{0, 1}},
        HuffmanEntry{'x', std::vector<bool>{0, 0, 1}},
        HuffmanEntry{'o', std::vector<bool>{0, 0, 0, 1}},
        HuffmanEntry{'e', std::vector<bool>{0, 0, 0, 0, 1}},
        HuffmanEntry{'a', std::vector<bool>{0, 0, 0, 0, 0}}};

    const std::string kExpected = "trxxaxetrorx";

    std::vector<bool> stream = ConcatenateSymbols(kTable, kExpected);

    koda::HuffmanDecoder decoder{kTable};

    std::string decoded;

    decoder(stream, decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(decoded, kExpected);
}
EndConstexprTest;

BeginConstexprTest(HuffmanDecoderTest, DecodeUniformDistribution) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    const koda::HuffmanTable<char> kTable = {
        HuffmanEntry{'a', std::vector<bool>{1, 0, 1}},
        HuffmanEntry{'b', std::vector<bool>{1, 0, 0}},
        HuffmanEntry{'c', std::vector<bool>{1, 1, 1}},
        HuffmanEntry{'d', std::vector<bool>{1, 1, 0}},
        HuffmanEntry{'e', std::vector<bool>{0, 0, 1}},
        HuffmanEntry{'f', std::vector<bool>{0, 0, 0}},
        HuffmanEntry{'g', std::vector<bool>{0, 1, 1}},
        HuffmanEntry{'h', std::vector<bool>{0, 1, 0}}};

    const std::string kExpected = "aghbcdefacbdfgghfead";

    std::vector<bool> stream = ConcatenateSymbols(kTable, kExpected);

    koda::HuffmanDecoder decoder{kTable};

    std::string decoded;

    decoder(stream, decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(decoded, kExpected);
}
EndConstexprTest;

BeginConstexprTest(HuffmanDecoderTest, DecodeDiracDistribution) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    const koda::HuffmanTable<char> kTable = {
        HuffmanEntry{'a', std::vector<bool>{}}};

    const std::string kExpected = "aaaaaaaaaaaaaaaaa";

    std::vector<bool> stream = ConcatenateSymbols(kTable, kExpected);

    koda::HuffmanDecoder decoder{kTable};

    std::string decoded;

    // It will yield infinitly otherwise!
    decoder(stream, decoded | koda::views::InsertFromBack |
                        koda::views::Take(kExpected.size()));

    ConstexprAssertEqual(decoded, kExpected);
}
EndConstexprTest;
