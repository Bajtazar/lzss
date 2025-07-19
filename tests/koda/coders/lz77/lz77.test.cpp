#include <koda/coders/lz77/lz77_decoder.hpp>
#include <koda/coders/lz77/lz77_encoder.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>

static constexpr const char* kTestString =
    "The number theoretic transform is based on generalizing the $ N$ th "
    "primitive root of unity (see §3.12) to a ``quotient ring'' instead of "
    "the usual field of complex numbers. Let $ W_N$ denote a primitive $ "
    "N$ th root of unity. We have been using $ W_N = \\exp(-j2\\pi/N)$ in "
    "the field of complex numbers, and it of course satisfies $ W_N^N=1$ , "
    "making it a root of unity; it also has the property that $ W_N^k$ "
    "visits all of the ``DFT frequency points'' on the unit circle in the "
    "$ z$ plane, as $ k$ goes from 0 to $ N-1$";

BeginConstexprTest(Lz77Test, NormalTest) {
    std::string sequence = kTestString;

    koda::Lz77Encoder<char> encoder{1024, 16};

    std::vector<uint8_t> encoded;

    encoder(sequence, encoded | koda::views::InsertFromBack |
                          koda::views::LittleEndianOutput);

    std::string decoded;

    koda::Lz77Decoder<char> decoder{1024, 16};

    decoder(sequence.size(), encoded | koda::views::LittleEndianInput,
            decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(sequence, decoded);
};
EndConstexprTest;

BeginConstexprTest(Lz77Test, SmallBufferTest) {
    std::string sequence = kTestString;

    koda::Lz77Encoder<char> encoder{1024, 1};

    std::vector<uint8_t> encoded;

    encoder(sequence, encoded | koda::views::InsertFromBack |
                          koda::views::LittleEndianOutput);

    std::string decoded;

    koda::Lz77Decoder<char> decoder{1024, 1};

    decoder(sequence.size(), encoded | koda::views::LittleEndianInput,
            decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(sequence, decoded);
};
EndConstexprTest;

BeginConstexprTest(Lz77Test, SmallDictionaryTest) {
    std::string sequence = kTestString;

    koda::Lz77Encoder<char> encoder{16, 16};

    std::vector<uint8_t> encoded;

    encoder(sequence, encoded | koda::views::InsertFromBack |
                          koda::views::LittleEndianOutput);

    std::string decoded;

    koda::Lz77Decoder<char> decoder{16, 16};

    decoder(sequence.size(), encoded | koda::views::LittleEndianInput,
            decoded | koda::views::InsertFromBack);

    ConstexprAssertEqual(sequence, decoded);
}
EndConstexprTest;
