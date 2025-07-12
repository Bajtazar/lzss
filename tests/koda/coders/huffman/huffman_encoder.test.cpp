#include <koda/coders/huffman/huffman_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <cassert>
#include <cmath>

static_assert(koda::Encoder<koda::HuffmanEncoder<uint8_t>, uint8_t>);

template <typename Token>
static constexpr std::vector<bool> ConcatenateSymbols(
    const koda::HuffmanTable<Token>& table, const auto& tokens) {
    std::vector<bool> result;
    for (const auto& token : tokens) {
        auto iter = table.Find(token);
        assert(iter != table.end());
        result.insert_range(result.end(), iter->second);
    }
    return result;
}

BeginConstexprTest(HuffmanEncoderTest, EncodeFirstScenario) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    const koda::HuffmanTable<uint32_t> kTable = {
        HuffmanEntry{0, std::vector<bool>{0}},
        HuffmanEntry{1, std::vector<bool>{1, 0, 1, 0}},
        HuffmanEntry{5, std::vector<bool>{1, 1, 1}},
        HuffmanEntry{16, std::vector<bool>{1, 1, 0}},
        HuffmanEntry{32, std::vector<bool>{1, 0, 0}},
        HuffmanEntry{43, std::vector<bool>{1, 0, 1, 1}}};

    std::vector<uint32_t> tokens = {0, 1, 1, 16, 0, 32, 1, 43};
    const std::vector<bool> kExpected = ConcatenateSymbols(kTable, tokens);
    ConstexprAssertTrue(kExpected.size() % CHAR_BIT == 0);

    koda::HuffmanEncoder encoder{kTable};

    std::vector<uint8_t> stream;

    encoder(tokens, stream | koda::views::InsertFromBack |
                        koda::views::LittleEndianOutput);

    ConstexprAssertEqual(stream | koda::views::LittleEndianInput, kExpected);
}
EndConstexprTest;

BeginConstexprTest(HuffmanEncoderTest, EncodeSecondScenario) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    const koda::HuffmanTable<char> kTable = {
        HuffmanEntry{'t', std::vector<bool>{1}},
        HuffmanEntry{'r', std::vector<bool>{0, 1}},
        HuffmanEntry{'x', std::vector<bool>{0, 0, 1}},
        HuffmanEntry{'o', std::vector<bool>{0, 0, 0, 1}},
        HuffmanEntry{'e', std::vector<bool>{0, 0, 0, 0, 1}},
        HuffmanEntry{'a', std::vector<bool>{0, 0, 0, 0, 0}}};

    std::string tokens = "trxxaxetrorx";
    const std::vector<bool> kExpected = ConcatenateSymbols(kTable, tokens);

    koda::HuffmanEncoder encoder{kTable};

    std::vector<char> stream;

    encoder(tokens, stream | koda::views::InsertFromBack |
                        koda::views::LittleEndianOutput);

    // call operator automatically flushes iter so check whether the flushed
    // size is valid
    ConstexprAssertEqual(stream.size(), std::ceil(kExpected.size() / 8.f));
    ConstexprAssertEqual(stream | koda::views::LittleEndianInput |
                             koda::views::Take(kExpected.size()),
                         kExpected);
}
EndConstexprTest;
