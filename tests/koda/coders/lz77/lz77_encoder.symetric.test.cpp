#include <koda/coders/lz77/lz77_encoder.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>

#include "common.hpp"

using DummyEncoder =
    Lz77DummyAuxEncoder<koda::Lz77IntermediateToken<char>, false>;

static_assert(koda::Encoder<koda::Lz77Encoder<char, DummyEncoder>, char>);

BeginConstexprTest(Lz77EncoderSymetricTest, EncodeTokens) {
    std::string input_sequence = "ala ma kota a kot ma ale";
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'a', 0, 0},  // 'a'
        koda::Lz77IntermediateToken<char>{'l', 0, 0},  // 'l'
        koda::Lz77IntermediateToken<char>{' ', 0, 1},  // 'a '
        koda::Lz77IntermediateToken<char>{'m', 0, 0},  // 'm'
        koda::Lz77IntermediateToken<char>{'k', 2, 2},  // 'a k'
        koda::Lz77IntermediateToken<char>{'o', 0, 0},  // 'o'
        koda::Lz77IntermediateToken<char>{'t', 0, 0},  // 't'
        koda::Lz77IntermediateToken<char>{'a', 5, 2},  // 'a a'
        koda::Lz77IntermediateToken<char>{' ', 6, 4},  // ' kot '
        koda::Lz77IntermediateToken<char>{'a', 4, 3},  // 'ma a'
        koda::Lz77IntermediateToken<char>{'e', 1, 1},  // 'le'
    };

    std::vector<uint8_t> target;

    koda::Lz77Encoder<char, DummyEncoder> encoder{1024, 4};
    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(Lz77EncoderSymetricTest, EncodeMoreTokens) {
    std::string input_sequence = "std::nullptr_t & nullptr";
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'s', 0, 0},   // 's'
        koda::Lz77IntermediateToken<char>{'t', 0, 0},   // 't'
        koda::Lz77IntermediateToken<char>{'d', 0, 0},   // 'd'
        koda::Lz77IntermediateToken<char>{':', 0, 0},   // ':'
        koda::Lz77IntermediateToken<char>{'n', 3, 1},   // ':n'
        koda::Lz77IntermediateToken<char>{'u', 0, 0},   // 'u'
        koda::Lz77IntermediateToken<char>{'l', 0, 0},   // 'l'
        koda::Lz77IntermediateToken<char>{'p', 7, 1},   // 'lp'
        koda::Lz77IntermediateToken<char>{'r', 1, 1},   // 'tr'
        koda::Lz77IntermediateToken<char>{'_', 0, 0},   // '_'
        koda::Lz77IntermediateToken<char>{' ', 10, 1},  // 't '
        koda::Lz77IntermediateToken<char>{'&', 0, 0},   // '&'
        koda::Lz77IntermediateToken<char>{'n', 14, 1},  // ' n'
        koda::Lz77IntermediateToken<char>{'r', 6, 5}    // 'ullptr'
    };

    std::vector<uint8_t> target;

    koda::Lz77Encoder<char, DummyEncoder> encoder{1024, 7};

    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(Lz77EncoderSymetricTest, EncodeTokensRepeatitions) {
    std::string input_sequence = "kot kot kot kot kot kot kot";
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'k', 0, 0},   // 'k'
        koda::Lz77IntermediateToken<char>{'o', 0, 0},   // 'o'
        koda::Lz77IntermediateToken<char>{'t', 0, 0},   // 't'
        koda::Lz77IntermediateToken<char>{' ', 0, 0},   // ' '
        koda::Lz77IntermediateToken<char>{' ', 0, 3},   // 'kot '
        koda::Lz77IntermediateToken<char>{' ', 4, 3},   // 'kot '
        koda::Lz77IntermediateToken<char>{' ', 8, 3},   // 'kot '
        koda::Lz77IntermediateToken<char>{' ', 12, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{' ', 16, 3},  // 'kot '
        koda::Lz77IntermediateToken<char>{'t', 20, 2}   // 'kot '
    };

    std::vector<uint8_t> target;

    koda::Lz77Encoder<char, DummyEncoder> encoder{1024, 3};
    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(Lz77EncoderSymetricTest, EncodeTokensShortDictionary) {
    std::string input_sequence = "kot abcdefghijkelmouprst kot";
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  //         [|kot]
        koda::Lz77IntermediateToken<char>{'o', 0, 0},  //        [k|ot ]
        koda::Lz77IntermediateToken<char>{'t', 0, 0},  //       [ko|t a]
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  //      [kot| ab]
        koda::Lz77IntermediateToken<char>{'a', 0, 0},  //     [kot |abc]
        koda::Lz77IntermediateToken<char>{'b', 0, 0},  //    [kot a|bcd]
        koda::Lz77IntermediateToken<char>{'c', 0, 0},  //   [kot ab|cde]
        koda::Lz77IntermediateToken<char>{'d', 0, 0},  //  [kot abc|def]
        koda::Lz77IntermediateToken<char>{'e', 0, 0},  // [kot abcd|efg]
        koda::Lz77IntermediateToken<char>{'f', 0, 0},  // [ot abcde|fgh]
        koda::Lz77IntermediateToken<char>{'g', 0, 0},  // [t abcdef|ghi]
        koda::Lz77IntermediateToken<char>{'h', 0, 0},  // [ abcdefg|hij]
        koda::Lz77IntermediateToken<char>{'i', 0, 0},  // [abcdefgh|ijk]
        koda::Lz77IntermediateToken<char>{'j', 0, 0},  // [bcdefghi|jke]
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  // [cdefghij|kel]
        koda::Lz77IntermediateToken<char>{'l', 1, 1},  // [defghijk|elm] e = 1,1
        koda::Lz77IntermediateToken<char>{'m', 0, 0},  // [fghijkel|mou]
        koda::Lz77IntermediateToken<char>{'o', 0, 0},  // [ghijkelm|oup]
        koda::Lz77IntermediateToken<char>{'u', 0, 0},  // [hijkelmo|upr]
        koda::Lz77IntermediateToken<char>{'p', 0, 0},  // [ijkelmou|prs]
        koda::Lz77IntermediateToken<char>{'r'},        // [jkelmoup|rst]
        koda::Lz77IntermediateToken<char>{'s', 0, 0},  // [kelmoupr|st ]
        koda::Lz77IntermediateToken<char>{'t', 0, 0},  // [elmouprs|t k]
        koda::Lz77IntermediateToken<char>{' ', 0, 0},  // [lmouprst| ko]
        koda::Lz77IntermediateToken<char>{'k', 0, 0},  // [mouprst |kot]
        koda::Lz77IntermediateToken<char>{'t', 0,
                                          1},  // [ouprst k|ot]  o = 0, 1
    };

    std::vector<uint8_t> target;

    koda::Lz77Encoder<char, DummyEncoder> encoder{8, 3};
    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(Lz77EncoderSymetricTest, EncodeRepeatingSequence) {
    std::string input_sequence = "aaaaaaa";
    std::vector<uint8_t> target;
    std::vector expected_result = {
        koda::Lz77IntermediateToken<char>{'a', 0, 0},
        koda::Lz77IntermediateToken<char>{'a', 0, 3},
        koda::Lz77IntermediateToken<char>{'a', 2, 1}};

    koda::Lz77Encoder<char, DummyEncoder> encoder{8, 3};
    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;
