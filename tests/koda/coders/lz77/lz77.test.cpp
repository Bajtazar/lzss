#include <koda/coders/huffman/huffman_decoder.hpp>
#include <koda/coders/huffman/huffman_encoder.hpp>
#include <koda/coders/lz77/lz77_decoder.hpp>
#include <koda/coders/lz77/lz77_encoder.hpp>
#include <koda/coders/lz77/lz77_intermediate_token_decoder.hpp>
#include <koda/coders/lz77/lz77_intermediate_token_encoder.hpp>
#include <koda/coders/rice/rice_decoder.hpp>
#include <koda/coders/rice/rice_encoder.hpp>
#include <koda/coders/uniform/uniform_decoder.hpp>
#include <koda/coders/uniform/uniform_encoder.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/counter.hpp>

static constexpr std::string_view kTestString =
    "The number theoretic transform is based on generalizing the $ N$ th "
    "primitive root of unity (see §3.12) to a ``quotient ring'' instead of "
    "the usual field of complex numbers. Let $ W_N$ denote a primitive $ "
    "N$ th root of unity. We have been using $ W_N = \\exp(-j2\\pi/N)$ in "
    "the field of complex numbers, and it of course satisfies $ W_N^N=1$ , "
    "making it a root of unity; it also has the property that $ W_N^k$ "
    "visits all of the ``DFT frequency points'' on the unit circle in the "
    "$ z$ plane, as $ k$ goes from 0 to $ N-1$";

static constexpr koda::HuffmanTable<char> BuildHuffmanTable() {
    return koda::MakeHuffmanTable(koda::Counter{kTestString}.counted());
}

using TokenEncoder = koda::HuffmanEncoder<char>;
using TokenDecoder = koda::HuffmanDecoder<char>;

using PositionEncoder = koda::UniformEncoder<uint32_t>;
using PositionDecoder = koda::UniformDecoder<uint32_t>;

using LengthEncoder = koda::RiceEncoder<uint16_t>;
using LengthDecoder = koda::RiceDecoder<uint16_t>;

using IMEncoder =
    koda::Lz77IntermediateTokenEncoder<char, uint32_t, uint16_t, TokenEncoder,
                                       PositionEncoder, LengthEncoder>;

using IMDecoder =
    koda::Lz77IntermediateTokenDecoder<char, uint32_t, uint16_t, TokenDecoder,
                                       PositionDecoder, LengthDecoder>;

using Lz77Encoder = koda::Lz77Encoder<char, IMEncoder>;
using Lz77Decoder = koda::Lz77Decoder<char, IMDecoder>;

BeginConstexprTest(Lz77Test, NormalTest) {
    const auto kHuffmanTable = BuildHuffmanTable();

    Lz77Encoder encoder{1024, 16,
                        IMEncoder{TokenEncoder{kHuffmanTable},
                                  PositionEncoder{10}, LengthEncoder{2}}};

    std::vector<uint8_t> encoded;

    encoder(kTestString, encoded | koda::views::InsertFromBack |
                             koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    std::string decoded;

    Lz77Decoder decoder{1024, 16,
                        IMDecoder{TokenDecoder{kHuffmanTable},
                                  PositionDecoder{10}, LengthDecoder{2}}};

    decoder(kTestString.size(), encoded | koda::views::LittleEndianInput,
            decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(kTestString, decoded);
};
EndConstexprTest;

BeginConstexprTest(Lz77Test, SmallBufferTest) {
    const auto kHuffmanTable = BuildHuffmanTable();

    Lz77Encoder encoder{1024, 16,
                        IMEncoder{TokenEncoder{kHuffmanTable},
                                  PositionEncoder{10}, LengthEncoder{1}}};

    std::vector<uint8_t> encoded;

    encoder(kTestString, encoded | koda::views::InsertFromBack |
                             koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    std::string decoded;

    Lz77Decoder decoder{1024, 16,
                        IMDecoder{TokenDecoder{kHuffmanTable},
                                  PositionDecoder{10}, LengthDecoder{1}}};

    decoder(kTestString.size(), encoded | koda::views::LittleEndianInput,
            decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(kTestString, decoded);
};
EndConstexprTest;

BeginConstexprTest(Lz77Test, SmallDictionaryTest) {
    const auto kHuffmanTable = BuildHuffmanTable();

    Lz77Encoder encoder{16, 16,
                        IMEncoder{TokenEncoder{kHuffmanTable},
                                  PositionEncoder{10}, LengthEncoder{3}}};

    std::vector<uint8_t> encoded;

    encoder(kTestString, encoded | koda::views::InsertFromBack |
                             koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    std::string decoded;

    Lz77Decoder decoder{16, 16,
                        IMDecoder{TokenDecoder{kHuffmanTable},
                                  PositionDecoder{10}, LengthDecoder{3}}};

    decoder(kTestString.size(), encoded | koda::views::LittleEndianInput,
            decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(kTestString, decoded);
}
EndConstexprTest;
