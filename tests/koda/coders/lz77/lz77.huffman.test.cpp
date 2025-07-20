#include <koda/coders/huffman/huffman_decoder.hpp>
#include <koda/coders/huffman/huffman_encoder.hpp>
#include <koda/coders/lz77/lz77_decoder.hpp>
#include <koda/coders/lz77/lz77_encoder.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/counter.hpp>

#include "common.hpp"

static constexpr std::string_view kTestString =
    "The number theoretic transform is based on generalizing the $ N$ th "
    "primitive root of unity (see §3.12) to a ``quotient ring'' instead of "
    "the usual field of complex numbers. Let $ W_N$ denote a primitive $ "
    "N$ th root of unity. We have been using $ W_N = \\exp(-j2\\pi/N)$ in "
    "the field of complex numbers, and it of course satisfies $ W_N^N=1$ , "
    "making it a root of unity; it also has the property that $ W_N^k$ "
    "visits all of the ``DFT frequency points'' on the unit circle in the "
    "$ z$ plane, as $ k$ goes from 0 to $ N-1$";

using HuffmanEncoder = koda::HuffmanEncoder<koda::Lz77IntermediateToken<char>>;
using HuffmanDecoder = koda::HuffmanDecoder<koda::Lz77IntermediateToken<char>>;
using Lz77Encoder = koda::Lz77Encoder<char, HuffmanEncoder>;
using Lz77Decoder = koda::Lz77Decoder<char, HuffmanDecoder>;

static constexpr koda::HuffmanTable<koda::Lz77IntermediateToken<char>>
BuildHuffmanTable(size_t dictionary_size, size_t look_ahead_size) {
    koda::Lz77Encoder<
        char, Lz77DummyAuxEncoder<koda::Lz77IntermediateToken<char>, false>>
        encoder{dictionary_size, look_ahead_size};

    std::vector<bool> encoded{0};
    encoder(kTestString, encoded);

    return koda::MakeHuffmanTable(
        koda::Counter{encoder.auxiliary_encoder().tokens}.counted());
}

BeginConstexprTest(Lz77HuffmanTest, NormalTest) {
    const auto kTable = BuildHuffmanTable(1024, 16);

    Lz77Encoder encoder{1024, 16, HuffmanEncoder{kTable}};

    std::vector<bool> encoded;

    encoder(kTestString, encoded | koda::views::InsertFromBack);

    std::string decoded;

    Lz77Decoder decoder{1024, 16, HuffmanDecoder{kTable}};

    decoder(kTestString.size(), encoded, decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(kTestString, decoded);
}
EndConstexprTest;

BeginConstexprTest(Lz77HuffmanTest, SmallBufferTest) {
    const auto kTable = BuildHuffmanTable(1024, 1);

    Lz77Encoder encoder{1024, 1, HuffmanEncoder{kTable}};

    std::vector<bool> encoded;

    encoder(kTestString, encoded | koda::views::InsertFromBack);

    std::string decoded;

    Lz77Decoder decoder{1024, 1, HuffmanDecoder{kTable}};

    decoder(kTestString.size(), encoded, decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(kTestString, decoded);
};
EndConstexprTest;

BeginConstexprTest(Lz77HuffmanTest, SmallDictionaryTest) {
    const auto kTable = BuildHuffmanTable(16, 16);

    Lz77Encoder encoder{16, 16, HuffmanEncoder{kTable}};

    std::vector<bool> encoded;

    encoder(kTestString, encoded | koda::views::InsertFromBack);

    std::string decoded;

    Lz77Decoder decoder{16, 16, HuffmanDecoder{kTable}};

    decoder(kTestString.size(), encoded, decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(kTestString, decoded);
}
EndConstexprTest;
