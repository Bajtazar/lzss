#include <koda/coders/lzss/lzss_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

namespace {
template <typename Tp>
struct LzssDummyAuxEncoder {
    bool should_pass_all_markers = true;
    std::vector<Tp> tokens = {};

    constexpr float TokenBitSize(Tp token) const {
        if (should_pass_all_markers) {
            return token.holds_symbol() ? 1.f : 0.f;  // save all markers
        }
        return koda::TokenTraits<Tp>::TokenBitSize(token);
    }

    constexpr void Encode(koda::InputRange<Tp> auto&& input,
                          [[maybe_unused]] koda::BitOutputRange auto&& output) {
        tokens.insert_range(tokens.end(), input);
    }

    constexpr void Flush([[maybe_unused]] koda::BitOutputRange auto&& output) {}

    constexpr void operator()(koda::InputRange<Tp> auto&& input,
                              koda::BitOutputRange auto&& output) {
        Encode(input, output);
    }
};

}  // namespace

BeginConstexprTest(LzssEncoder, EncodeTokens) {
    std::string input_sequence = "ala ma kota a kot ma ale";
    std::vector expected_result = {
        koda::LzssIntermediateToken<char>{'a'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{0,
                                          1},  // similar to LZ77 in this mode!
        koda::LzssIntermediateToken<char>{' '},
        koda::LzssIntermediateToken<char>{'m'},
        koda::LzssIntermediateToken<char>{2, 2},  // 'a '
        koda::LzssIntermediateToken<char>{'k'},
        koda::LzssIntermediateToken<char>{'o'},
        koda::LzssIntermediateToken<char>{'t'},
        koda::LzssIntermediateToken<char>{5, 2},  // 'a '
        koda::LzssIntermediateToken<char>{5, 4},  // 'a ko'
        koda::LzssIntermediateToken<char>{9, 1},  // 't'
        koda::LzssIntermediateToken<char>{3, 4},  // ' ma '
        koda::LzssIntermediateToken<char>{0, 2},  // 'al'
        koda::LzssIntermediateToken<char>{'e'}};

    std::vector<uint8_t> target;
    auto source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange output_range{koda::LittleEndianOutputBitIter{source},
                                       std::default_sentinel};

    koda::LzssEncoder<char,
                      LzssDummyAuxEncoder<koda::LzssIntermediateToken<char>>>
        encoder{1024, 4};
    encoder(input_sequence, output_range);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(LzssEncoder, EncodeTokensRealScenario) {
    std::string input_sequence = "std::nullptr_t & nullptr";
    std::vector expected_result = {
        koda::LzssIntermediateToken<char>{'s'},
        koda::LzssIntermediateToken<char>{'t'},
        koda::LzssIntermediateToken<char>{'d'},
        koda::LzssIntermediateToken<char>{':'},
        koda::LzssIntermediateToken<char>{':'},
        koda::LzssIntermediateToken<char>{'n'},
        koda::LzssIntermediateToken<char>{'u'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{'p'},
        koda::LzssIntermediateToken<char>{'t'},
        koda::LzssIntermediateToken<char>{'r'},
        koda::LzssIntermediateToken<char>{'_'},
        koda::LzssIntermediateToken<char>{'t'},
        koda::LzssIntermediateToken<char>{' '},
        koda::LzssIntermediateToken<char>{'&'},
        koda::LzssIntermediateToken<char>{' '},
        koda::LzssIntermediateToken<char>{5, 7},  // 'nullptr'
    };

    std::vector<uint8_t> target;
    auto source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange output_range{koda::LittleEndianOutputBitIter{source},
                                       std::default_sentinel};

    koda::LzssEncoder<char,
                      LzssDummyAuxEncoder<koda::LzssIntermediateToken<char>>>
        encoder{1024, 7,
                LzssDummyAuxEncoder<koda::LzssIntermediateToken<char>>{false}};
    encoder(input_sequence, output_range);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(LzssEncoder, EncodeTokensRealScenarioTooShortLookAhead) {
    std::string input_sequence = "std::nullptr_t & nullptr";
    std::vector expected_result = {
        koda::LzssIntermediateToken<char>{'s'},
        koda::LzssIntermediateToken<char>{'t'},
        koda::LzssIntermediateToken<char>{'d'},
        koda::LzssIntermediateToken<char>{':'},
        koda::LzssIntermediateToken<char>{':'},
        koda::LzssIntermediateToken<char>{'n'},
        koda::LzssIntermediateToken<char>{'u'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{'p'},
        koda::LzssIntermediateToken<char>{'t'},
        koda::LzssIntermediateToken<char>{'r'},
        koda::LzssIntermediateToken<char>{'_'},
        koda::LzssIntermediateToken<char>{'t'},
        koda::LzssIntermediateToken<char>{' '},
        koda::LzssIntermediateToken<char>{'&'},
        koda::LzssIntermediateToken<char>{' '},
        koda::LzssIntermediateToken<char>{'n'},
        koda::LzssIntermediateToken<char>{'u'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{'p'},
        koda::LzssIntermediateToken<char>{'t'},
        koda::LzssIntermediateToken<char>{'r'},
    };

    std::vector<uint8_t> target;
    auto source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange output_range{koda::LittleEndianOutputBitIter{source},
                                       std::default_sentinel};

    koda::LzssEncoder<char,
                      LzssDummyAuxEncoder<koda::LzssIntermediateToken<char>>>
        encoder{1024, 4,
                LzssDummyAuxEncoder<koda::LzssIntermediateToken<char>>{false}};
    encoder(input_sequence, output_range);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(LzssEncoder, EncodeTokensRepeatitions) {
    std::string input_sequence = "kot kot kot kot kot kot kot";
    std::vector expected_result = {
        koda::LzssIntermediateToken<char>{'k'},
        koda::LzssIntermediateToken<char>{'o'},
        koda::LzssIntermediateToken<char>{'t'},
        koda::LzssIntermediateToken<char>{' '},
        koda::LzssIntermediateToken<char>{0, 3},   // 'kot'
        koda::LzssIntermediateToken<char>{3, 3},   // ' ko'
        koda::LzssIntermediateToken<char>{6, 3},   // 't k'
        koda::LzssIntermediateToken<char>{9, 3},   // 'ot '
        koda::LzssIntermediateToken<char>{12, 3},  // 'kot'
        koda::LzssIntermediateToken<char>{15, 3},  // ' ko'
        koda::LzssIntermediateToken<char>{18, 3},  // 't k'
        koda::LzssIntermediateToken<char>{21, 2},  // 'ot'
    };

    std::vector<uint8_t> target;
    auto source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange output_range{koda::LittleEndianOutputBitIter{source},
                                       std::default_sentinel};

    koda::LzssEncoder<char,
                      LzssDummyAuxEncoder<koda::LzssIntermediateToken<char>>>
        encoder{1024, 3};
    encoder(input_sequence, output_range);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;

BeginConstexprTest(LzssEncoder, EncodeTokensShortDictionary) {
    std::string input_sequence = "kot abcdefghijkelmouprst kot";
    std::vector expected_result = {
        koda::LzssIntermediateToken<char>{'k'},   //         [|kot]
        koda::LzssIntermediateToken<char>{'o'},   //        [k|ot ]
        koda::LzssIntermediateToken<char>{'t'},   //       [ko|t a]
        koda::LzssIntermediateToken<char>{' '},   //      [kot| ab]
        koda::LzssIntermediateToken<char>{'a'},   //     [kot |abc]
        koda::LzssIntermediateToken<char>{'b'},   //    [kot a|bcd]
        koda::LzssIntermediateToken<char>{'c'},   //   [kot ab|cde]
        koda::LzssIntermediateToken<char>{'d'},   //  [kot abc|def]
        koda::LzssIntermediateToken<char>{'e'},   // [kot abcd|efg]
        koda::LzssIntermediateToken<char>{'f'},   // [ot abcde|fgh]
        koda::LzssIntermediateToken<char>{'g'},   // [t abcdef|ghi]
        koda::LzssIntermediateToken<char>{'h'},   // [ abcdefg|hij]
        koda::LzssIntermediateToken<char>{'i'},   // [abcdefgh|ijk]
        koda::LzssIntermediateToken<char>{'j'},   // [bcdefghi|jke]
        koda::LzssIntermediateToken<char>{'k'},   // [cdefghij|kel]
        koda::LzssIntermediateToken<char>{1, 1},  // [defghijk|elm] e = 1,1
        koda::LzssIntermediateToken<char>{'l'},   // [efghijke|lmo]
        koda::LzssIntermediateToken<char>{'m'},   // [fghijkel|mou]
        koda::LzssIntermediateToken<char>{'o'},   // [ghijkelm|oup]
        koda::LzssIntermediateToken<char>{'u'},   // [hijkelmo|upr]
        koda::LzssIntermediateToken<char>{'p'},   // [ijkelmou|prs]
        koda::LzssIntermediateToken<char>{'r'},   // [jkelmoup|rst]
        koda::LzssIntermediateToken<char>{'s'},   // [kelmoupr|st ]
        koda::LzssIntermediateToken<char>{'t'},   // [elmouprs|t k]
        koda::LzssIntermediateToken<char>{' '},   // [lmouprst| ko]
        koda::LzssIntermediateToken<char>{'k'},   // [mouprst |kot]
        koda::LzssIntermediateToken<char>{0, 1},  // [ouprst k|ot]  o = 0, 1
        koda::LzssIntermediateToken<char>{4, 1}   // [uprst ko|t]   t = 4,1
    };

    std::vector<uint8_t> target;
    auto source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange output_range{koda::LittleEndianOutputBitIter{source},
                                       std::default_sentinel};

    koda::LzssEncoder<char,
                      LzssDummyAuxEncoder<koda::LzssIntermediateToken<char>>>
        encoder{8, 3};
    encoder(input_sequence, output_range);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
}
EndConstexprTest;
