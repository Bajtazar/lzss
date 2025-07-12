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

BeginConstexprTest(HuffmanEncoderTest, EncodeRandomDistribution) {
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
                        koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    ConstexprAssertEqual(stream | koda::views::LittleEndianInput, kExpected);
}
EndConstexprTest;

BeginConstexprTest(HuffmanEncoderTest, EncodeGeometricDistribution) {
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
                        koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    // call operator automatically flushes iter so check whether the flushed
    // size is valid
    ConstexprAssertEqual(stream.size(), std::ceil(kExpected.size() / 8.f));
    ConstexprAssertEqual(stream | koda::views::LittleEndianInput |
                             koda::views::Take(kExpected.size()),
                         kExpected);
}
EndConstexprTest;

BeginConstexprTest(HuffmanEncoderTest, EncodeUniformDistribution) {
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

    std::string tokens = "aghbcdefacbdfgghfead";
    const std::vector<bool> kExpected = ConcatenateSymbols(kTable, tokens);

    koda::HuffmanEncoder encoder{kTable};

    std::vector<char> stream;

    encoder(tokens, stream | koda::views::InsertFromBack |
                        koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    // call operator automatically flushes iter so check whether the flushed
    // size is valid
    ConstexprAssertEqual(stream.size(), std::ceil(kExpected.size() / 8.f));
    ConstexprAssertEqual(stream | koda::views::LittleEndianInput |
                             koda::views::Take(kExpected.size()),
                         kExpected);
}
EndConstexprTest;

BeginConstexprTest(HuffmanEncoderTest, EncodeDiracDistribution) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    const koda::HuffmanTable<char> kTable = {
        HuffmanEntry{'a', std::vector<bool>{}}};

    std::string tokens = "aaaaaaaaaaaaaaaaa";
    const std::vector<bool> kExpected{};

    koda::HuffmanEncoder encoder{kTable};

    std::vector<char> stream;

    encoder(tokens, stream | koda::views::InsertFromBack |
                        koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    ConstexprAssertTrue(stream.empty());
}
EndConstexprTest;

BeginConstexprTest(HuffmanEncoderTest, PartialInputEncoding) {
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

    std::vector<bool> stream;

    encoder.EncodeN(5, tokens, stream | koda::views::InsertFromBack);

    ConstexprAssertEqual(
        stream, ConcatenateSymbols(kTable, tokens | koda::views::Take(5)));

    encoder.EncodeN(2, tokens | std::views::drop(5),
                    stream | koda::views::InsertFromBack);

    ConstexprAssertEqual(
        stream, ConcatenateSymbols(kTable, tokens | koda::views::Take(7)));

    encoder(tokens | std::views::drop(7), stream | koda::views::InsertFromBack);

    ConstexprAssertEqual(stream.size(), kExpected.size());
    ConstexprAssertEqual(stream, kExpected);
}
EndConstexprTest;

// BeginConstexprTest(HuffmanEncoderTest, PartialOutputEncoding) {
//     using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

//     const koda::HuffmanTable<char> kTable = {
//         HuffmanEntry{'t', std::vector<bool>{1}},
//         HuffmanEntry{'r', std::vector<bool>{0, 1}},
//         HuffmanEntry{'x', std::vector<bool>{0, 0, 1}},
//         HuffmanEntry{'o', std::vector<bool>{0, 0, 0, 1}},
//         HuffmanEntry{'e', std::vector<bool>{0, 0, 0, 0, 1}},
//         HuffmanEntry{'a', std::vector<bool>{0, 0, 0, 0, 0}}};

//     std::string tokens = "trxxaxetrorx";
//     const std::vector<bool> kExpected = ConcatenateSymbols(kTable, tokens);

//     koda::HuffmanEncoder encoder{kTable};

//     std::vector<bool> stream;

//     auto [in_1, _] =
//         encoder.EncodeN(5, tokens, stream | koda::views::InsertFromBack);

//     ConstexprAssertEqual(stream.size(), 5);
//     ConstexprAssertEqual(stream, kExpected | koda::views::Take(5));

//     auto [in_2, _] = encoder.EncodeN(13, std::move(in_1),
//                                      stream | koda::views::InsertFromBack);

//     ConstexprAssertEqual(stream.size(), 18);
//     ConstexprAssertEqual(stream, kExpected | koda::views::Take(18));

//     encoder(std::move(in_2), stream | koda::views::InsertFromBack);

//     ConstexprAssertEqual(stream.size(), kExpected.size());
//     ConstexprAssertEqual(stream, kExpected);
// }
// EndConstexprTest;
