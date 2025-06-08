#include <koda/coders/lzss/lzss_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

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
EndConstexprTest(LzssEncoder, EncodeTokens);

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
EndConstexprTest(LzssEncoder, EncodeTokensRealScenario);

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
EndConstexprTest(LzssEncoder, EncodeTokensRealScenarioTooShortLookAhead);

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
EndConstexprTest(LzssEncoder, EncodeTokensRepeatitions);
