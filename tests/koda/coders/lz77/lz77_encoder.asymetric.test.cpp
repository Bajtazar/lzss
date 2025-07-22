#include <koda/coders/lz77/lz77_encoder.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>

#include <gtest/gtest.h>

#include <print>

#include "common.hpp"

using DummyEncoder =
    Lz77DummyAuxEncoder<koda::Lz77IntermediateToken<char>, true>;

static_assert(koda::Encoder<koda::Lz77Encoder<char, DummyEncoder>, char>);

BeginConstexprTest(Lz77EncoderAsymetricTest, EncodeTokens) {
    std::string input_sequence = "ala ma kota a kot ma ale";
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'a', 1, 1},  // 'al'
        koda::Lz77IntermediateToken<char>{'a', 3, 4},  // 'a ma '
        koda::Lz77IntermediateToken<char>{'k', 7, 2},  // 'kot'
        koda::Lz77IntermediateToken<char>{'a', 2, 2},  // 'a a'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  // ' '
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  // ' '
        koda::Lz77IntermediateToken<char>{'o', 0, 0},  // 'o'
        koda::Lz77IntermediateToken<char>{'t', 3, 1},  // 't '
        koda::Lz77IntermediateToken<char>{'m', 2, 1},  // 'ma'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  // ' '
        koda::Lz77IntermediateToken<char>{'a', 0, 0},  // 'a'
        koda::Lz77IntermediateToken<char>{'l', 0, 0},  // 'l'
        koda::Lz77IntermediateToken<char>{'e', 0, 0},  // 'e'
    };

    std::vector<uint8_t> target;

    koda::Lz77Encoder<char, DummyEncoder> encoder{1024, 4};
    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(Lz77EncoderAsymetricTest, EncodeMoreTokens) {
    std::string input_sequence = "std::nullptr_t & nullptr";
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'s', 1, 1},   // 'st'
        koda::Lz77IntermediateToken<char>{'d', 19, 1},  // 'd:'
        koda::Lz77IntermediateToken<char>{':', 0, 7},   // 'nullptr'
        koda::Lz77IntermediateToken<char>{'_', 1, 1},   // '_t'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},   // ' '
        koda::Lz77IntermediateToken<char>{'&', 0, 0},   // '&'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},   // ' '
        koda::Lz77IntermediateToken<char>{'n', 0, 0},   // 'n'
        koda::Lz77IntermediateToken<char>{'u', 3, 1},   // 'ul'
        koda::Lz77IntermediateToken<char>{'l', 0, 0},   // 'l'
        koda::Lz77IntermediateToken<char>{'p', 0, 0},   // 'p'
        koda::Lz77IntermediateToken<char>{'t', 0, 0},   // 't'
        koda::Lz77IntermediateToken<char>{'r', 0, 0},   // 'r'
    };

    std::vector<uint8_t> target;

    koda::Lz77Encoder<char, DummyEncoder> encoder{1024, 7};

    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(Lz77EncoderAsymetricTest, EncodeTokensRepeatitions) {
    std::string input_sequence = "kot kot kot kot kot kot kot";
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'k', 3, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'k', 0, 2},  // 'kot'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  // ' '
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  // 'k'
        koda::Lz77IntermediateToken<char>{'o', 0, 0},  // 'o'
        koda::Lz77IntermediateToken<char>{'t', 0, 0}   // 't'
    };

    std::vector<uint8_t> target;

    koda::Lz77Encoder<char, DummyEncoder> encoder{1024, 3};
    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(Lz77EncoderAsymetricTest, EncodeTokensShortDictionary) {
    std::string input_sequence = "kot abcdefghijkelmouprst kot";
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  // [kot| abcdefg]
        koda::Lz77IntermediateToken<char>{'o', 0, 0},  // [ot |abcdefgh]
        koda::Lz77IntermediateToken<char>{'t', 0, 0},  // [t a|bcdefghi]
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  // [ ab|cdefghij]
        koda::Lz77IntermediateToken<char>{'a', 0, 0},  // [abc|defghijk]
        koda::Lz77IntermediateToken<char>{'b', 0, 0},  // [bcd|efghijke]
        koda::Lz77IntermediateToken<char>{'c', 0, 0},  // [cde|fghijkel]
        koda::Lz77IntermediateToken<char>{'d', 2, 1},  // [def|ghijkelm] e=2,1
        koda::Lz77IntermediateToken<char>{'f', 0, 0},  // [fg|hijkelmou]
        koda::Lz77IntermediateToken<char>{'g', 0, 0},  // [ghi|jkelmoup]
        koda::Lz77IntermediateToken<char>{'h', 0, 0},  // [hij|kelmoupr]
        koda::Lz77IntermediateToken<char>{'i', 0, 0},  // [ijk|elmouprs]
        koda::Lz77IntermediateToken<char>{'j', 0, 0},  // [jke|lmouprst]
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  // [kel|mouprst ]
        koda::Lz77IntermediateToken<char>{'e', 0, 0},  // [elm|ouprst k]
        koda::Lz77IntermediateToken<char>{'l', 0, 0},  // [lmo|uprst ko]
        koda::Lz77IntermediateToken<char>{'m', 1, 1},  // [mou|prst kot] o=1,1
        koda::Lz77IntermediateToken<char>{'u', 0, 0},  // [upr|st kot]
        koda::Lz77IntermediateToken<char>{'p', 0, 0},  // [prs|t kot]
        koda::Lz77IntermediateToken<char>{'r', 0, 0},  // [rst| kot]
        koda::Lz77IntermediateToken<char>{'s', 0, 1},  // [st |kot]      t=0,1
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  // [ ko|t]
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  // [kot]
        koda::Lz77IntermediateToken<char>{'o', 0, 0},  // [ot]
        koda::Lz77IntermediateToken<char>{'t', 0, 0},  // [o]
    };

    std::vector<uint8_t> target;

    koda::Lz77Encoder<char, DummyEncoder> encoder{8, 3};
    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(Lz77EncoderAsymetricTest, EncodeRepeatingSequence) {
    std::string input_sequence = "aaaaaaa";
    std::vector<uint8_t> target;
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'a', 0, 3},
        koda::Lz77IntermediateToken<char>{'a', 0, 1},
        koda::Lz77IntermediateToken<char>{'a', 0, 0}};

    koda::Lz77Encoder<char, DummyEncoder> encoder{8, 3};
    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;
