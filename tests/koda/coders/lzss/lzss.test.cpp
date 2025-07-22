#include <koda/coders/huffman/huffman_decoder.hpp>
#include <koda/coders/huffman/huffman_encoder.hpp>
#include <koda/coders/lzss/lzss_decoder.hpp>
#include <koda/coders/lzss/lzss_encoder.hpp>
#include <koda/coders/lzss/lzss_intermediate_token_decoder.hpp>
#include <koda/coders/lzss/lzss_intermediate_token_encoder.hpp>
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

using LengthEncoder = koda::UniformEncoder<uint16_t>;
using LengthDecoder = koda::UniformDecoder<uint16_t>;

using IMEncoder =
    koda::LzssIntermediateTokenEncoder<char, uint32_t, uint16_t, TokenEncoder,
                                       PositionEncoder, LengthEncoder>;

using IMDecoder =
    koda::LzssIntermediateTokenDecoder<char, uint32_t, uint16_t, TokenDecoder,
                                       PositionDecoder, LengthDecoder>;

using LzssEncoder = koda::LzssEncoder<char, IMEncoder>;
using LzssDecoder = koda::LzssDecoder<char, IMDecoder>;

BeginConstexprTest(LzssTest, NormalTest) {
    const auto kHuffmanTable = BuildHuffmanTable();
    LzssEncoder encoder{1024, 16,
                        IMEncoder{TokenEncoder{kHuffmanTable},
                                  PositionEncoder{10}, LengthEncoder{4}}};

    std::vector<uint8_t> encoded;

    encoder(kTestString, encoded | koda::views::InsertFromBack |
                             koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    std::string decoded;

    LzssDecoder decoder{1024, 16,
                        IMDecoder{TokenDecoder{kHuffmanTable},
                                  PositionDecoder{10}, LengthDecoder{4}}};

    decoder(kTestString.size(), encoded | koda::views::LittleEndianInput,
            decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(kTestString, decoded);
};
EndConstexprTest;

BeginConstexprTest(LzssTest, SmallBufferTest) {
    const auto kHuffmanTable = BuildHuffmanTable();
    LzssEncoder encoder{1024, 1,
                        IMEncoder{TokenEncoder{kHuffmanTable},
                                  PositionEncoder{10}, LengthEncoder{0}}};

    std::vector<uint8_t> encoded;

    encoder(kTestString, encoded | koda::views::InsertFromBack |
                             koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    std::string decoded;

    LzssDecoder decoder{1024, 1,
                        IMDecoder{TokenDecoder{kHuffmanTable},
                                  PositionDecoder{10}, LengthDecoder{0}}};

    decoder(kTestString.size(), encoded | koda::views::LittleEndianInput,
            decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(kTestString, decoded);
};
EndConstexprTest;

BeginConstexprTest(LzssTest, SmallDictionaryTest) {
    const auto kHuffmanTable = BuildHuffmanTable();
    LzssEncoder encoder{16, 16,
                        IMEncoder{TokenEncoder{kHuffmanTable},
                                  PositionEncoder{4}, LengthEncoder{4}}};

    std::vector<uint8_t> encoded;

    encoder(kTestString, encoded | koda::views::InsertFromBack |
                             koda::views::LittleEndianOutput)
        .output_range.begin()
        .Flush();

    std::string decoded;

    LzssDecoder decoder{16, 16,
                        IMDecoder{TokenDecoder{kHuffmanTable},
                                  PositionDecoder{4}, LengthDecoder{4}}};

    decoder(kTestString.size(), encoded | koda::views::LittleEndianInput,
            decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(kTestString, decoded);
};
EndConstexprTest;
