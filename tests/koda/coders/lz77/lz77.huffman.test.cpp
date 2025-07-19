#include <koda/coders/huffman/huffman_decoder.hpp>
#include <koda/coders/huffman/huffman_encoder.hpp>
#include <koda/coders/lz77/lz77_decoder.hpp>
#include <koda/coders/lz77/lz77_encoder.hpp>
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

using HuffmanEncoder = koda::HuffmanEncoder<koda::Lz77IntermediateToken<char>>;
using HuffmanDecoder = koda::HuffmanDecoder<koda::Lz77IntermediateToken<char>>;
using Lz77Encoder = koda::Lz77Encoder<char, HuffmanEncoder>;
using Lz77Decoder = koda::Lz77Decoder<char, HuffmanDecoder>;

namespace {

template <typename Tp>
struct Lz77DummyAuxEncoder
    : public koda::EncoderInterface<Tp, Lz77DummyAuxEncoder<Tp>> {
    constexpr explicit Lz77DummyAuxEncoder() = default;

    std::vector<Tp> tokens = {};

    [[noreturn]] constexpr float TokenBitSize([[maybe_unused]] Tp token) const {
        throw std::runtime_error{"Unused expression!"};
    }

    constexpr auto Encode(koda::InputRange<Tp> auto&& input,
                          koda::BitOutputRange auto&& output) {
        tokens.insert_range(tokens.end(), std::forward<decltype(input)>(input));
        // Circumvent static analysis
        auto iter = std::ranges::begin(input);
        const auto sent = std::ranges::end(input);
        for (; iter != sent; ++iter);
        return koda::CoderResult{iter, sent,
                                 std::forward<decltype(output)>(output)};
    }

    constexpr auto Flush(koda::BitOutputRange auto&& output) {
        return std::forward<decltype(output)>(output);
    }
};

constexpr koda::HuffmanTable<koda::Lz77IntermediateToken<char>>
BuildHuffmanTable(size_t dictionary_size, size_t look_ahead_size) {
    koda::Lz77Encoder<char,
                      Lz77DummyAuxEncoder<koda::Lz77IntermediateToken<char>>>
        encoder{dictionary_size, look_ahead_size};

    std::vector<bool> encoded{0};
    encoder(kTestString, encoded);

    return koda::MakeHuffmanTable(
        koda::Counter{encoder.auxiliary_encoder().tokens}.counted());
}

}  // namespace

BeginConstexprTest(Lz77Test, NormalTest) {
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

// BeginConstexprTest(Lz77Test, SmallBufferTest) {
//     std::string sequence = kTestString;

//     koda::Lz77Encoder<char> encoder{1024, 1};

//     std::vector<uint8_t> encoded;

//     encoder(sequence, encoded | koda::views::InsertFromBack |
//                           koda::views::LittleEndianOutput);

//     std::string decoded;

//     koda::Lz77Decoder<char> decoder{1024, 1};

//     decoder(sequence.size(), encoded | koda::views::LittleEndianInput,
//             decoded | koda::views::InsertFromBack);

//     ConstexprAssertEqual(sequence, decoded);
// };
// EndConstexprTest;

// BeginConstexprTest(Lz77Test, SmallDictionaryTest) {
//     std::string sequence = kTestString;

//     koda::Lz77Encoder<char> encoder{16, 16};

//     std::vector<uint8_t> encoded;

//     encoder(sequence, encoded | koda::views::InsertFromBack |
//                           koda::views::LittleEndianOutput);

//     std::string decoded;

//     koda::Lz77Decoder<char> decoder{16, 16};

//     decoder(sequence.size(), encoded | koda::views::LittleEndianInput,
//             decoded | koda::views::InsertFromBack);

//     ConstexprAssertEqual(sequence, decoded);
// }
// EndConstexprTest;
