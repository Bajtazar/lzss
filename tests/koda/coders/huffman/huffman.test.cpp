#include <koda/coders/huffman/huffman_decoder.hpp>
#include <koda/coders/huffman/huffman_encoder.hpp>
#include <koda/coders/huffman/huffman_table.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>
#include <koda/utils/counter.hpp>

#include <string_view>

static constexpr std::string_view kTestString =
    "The number theoretic transform is based on generalizing the $ N$ th "
    "primitive root of unity (see §3.12) to a ``quotient ring'' instead of "
    "the usual field of complex numbers. Let $ W_N$ denote a primitive $ "
    "N$ th root of unity. We have been using $ W_N = \\exp(-j2\\pi/N)$ in "
    "the field of complex numbers, and it of course satisfies $ W_N^N=1$ , "
    "making it a root of unity; it also has the property that $ W_N^k$ "
    "visits all of the ``DFT frequency points'' on the unit circle in the "
    "$ z$ plane, as $ k$ goes from 0 to $ N-1$";

BeginConstexprTest(HuffmanTest, NormalTest) {
    const auto kCount = koda::Counter{kTestString}.counted();
    const auto kTable = koda::MakeHuffmanTable(kCount);

    std::vector<bool> stream;
    std::string result;

    koda::HuffmanEncoder encoder{kTable};
    koda::HuffmanDecoder decoder{kTable};

    encoder(kTestString, stream | koda::views::InsertFromBack);

    decoder(stream, result | koda::views::InsertFromBack);

    ConstexprAssertEqual(result, kTestString);
}
EndConstexprTest;
