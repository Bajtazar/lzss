#include <koda/coders/lz77/lz77_decoder.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>

#include "common.hpp"

using DummyDecoder =
    Lz77DummyAuxDecoder<koda::Lz77IntermediateToken<char>, true>;

static_assert(koda::Decoder<koda::Lz77Decoder<char, DummyDecoder>, char>);

using namespace std::literals;

BeginConstexprTest(Lz77DecoderAsymetricTest, DecodeTokens) {
    std::vector input_sequence = {
        koda::Lz77IntermediateToken<char>{'e', 0, 0},  // 'e'
        koda::Lz77IntermediateToken<char>{'l', 0, 0},  // 'l'
        koda::Lz77IntermediateToken<char>{'a', 0, 0},  // 'a'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  // ' '
        koda::Lz77IntermediateToken<char>{'m', 2, 1},  // 'ma'
        koda::Lz77IntermediateToken<char>{'t', 3, 1},  // 't '
        koda::Lz77IntermediateToken<char>{'o', 0, 0},  // 'o'
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  // ' '
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  // ' '
        koda::Lz77IntermediateToken<char>{'a', 2, 2},  // 'a a'
        koda::Lz77IntermediateToken<char>{'k', 7, 2},  // 'kot'
        koda::Lz77IntermediateToken<char>{'a', 3, 4},  // 'a ma '
        koda::Lz77IntermediateToken<char>{'a', 1, 1},  // 'al'
    };
    std::string kExpected{std::from_range,
                          "ala ma kota a kot ma ale"sv | std::views::reverse};
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::Lz77Decoder<char, DummyDecoder> decoder{
        1024, 4, DummyDecoder{std::move(input_sequence)}};

    decoder(binary_range | koda::views::LittleEndianInput,
            target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, kExpected);
}
EndConstexprTest;

BeginConstexprTest(Lz77DecoderAsymetricTest, DecodeMoreTokens) {
    std::vector input_sequence = {
        koda::Lz77IntermediateToken<char>{'r', 0, 0},   // 'r'
        koda::Lz77IntermediateToken<char>{'t', 0, 0},   // 't'
        koda::Lz77IntermediateToken<char>{'p', 0, 0},   // 'p'
        koda::Lz77IntermediateToken<char>{'l', 0, 0},   // 'l'
        koda::Lz77IntermediateToken<char>{'u', 3, 1},   // 'ul'
        koda::Lz77IntermediateToken<char>{'n', 0, 0},   // 'n'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},   // ' '
        koda::Lz77IntermediateToken<char>{'&', 0, 0},   // '&'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},   // ' '
        koda::Lz77IntermediateToken<char>{'_', 1, 1},   // '_t'
        koda::Lz77IntermediateToken<char>{':', 0, 7},   // 'nullptr'
        koda::Lz77IntermediateToken<char>{'d', 19, 1},  // 'd:'
        koda::Lz77IntermediateToken<char>{'s', 1, 1},   // 'st'
    };
    std::string kExpected{std::from_range,
                          "std::nullptr_t & nullptr"sv | std::views::reverse};
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::Lz77Decoder<char, DummyDecoder> decoder{
        1024, 7, DummyDecoder{std::move(input_sequence)}};

    decoder(binary_range | koda::views::LittleEndianInput,
            target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, kExpected);
}
EndConstexprTest;

BeginConstexprTest(Lz77DecoderAsymetricTest, DecodeTokensRepeatitions) {
    std::vector input_sequence = {
        koda::Lz77IntermediateToken<char>{'t', 0, 0},  // 't'
        koda::Lz77IntermediateToken<char>{'o', 0, 0},  // 'o'
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  // 'k'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  // ' '
        koda::Lz77IntermediateToken<char>{'k', 0, 2},  // 'kot'
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
    };
    std::string kExpected{
        std::from_range, "kot kot kot kot kot kot kot"sv | std::views::reverse};
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::Lz77Decoder<char, DummyDecoder> decoder{
        1024, 3, DummyDecoder{std::move(input_sequence)}};

    decoder(binary_range | koda::views::LittleEndianInput,
            target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, kExpected);
}
EndConstexprTest;

BeginConstexprTest(Lz77DecoderAsymetricTest, DecodeRepeatingSequence) {
    std::vector input_sequence = {
        koda::Lz77IntermediateToken<char>{'a', 0, 0},
        koda::Lz77IntermediateToken<char>{'a', 0, 1},
        koda::Lz77IntermediateToken<char>{'a', 0, 3},
    };
    std::string kExpected{std::from_range, "aaaaaaa"sv};
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::Lz77Decoder<char, DummyDecoder> decoder{
        8, 3, DummyDecoder{std::move(input_sequence)}};

    decoder(binary_range | koda::views::LittleEndianInput,
            target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, kExpected);
}
EndConstexprTest;
