#include <koda/coders/lzss/lzss_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

namespace {
template <typename Tp>
struct LzssDummyAuxEncoder {
    std::vector<Tp> tokens = {};

    constexpr float TokenBitSize(Tp token) const {
        return token.holds_symbol() ? 1.f : 0.f;  // save all markers
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

BeginConstexprTest(LzssEncoder, EncodeBytes) {
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
};
EndConstexprTest(LzssEncoder, EncodeBytes);
