#include <koda/coders/lzss/lzss_decoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

namespace {

template <typename Tp>
struct LzssDummyAuxDecoder
    : public koda::DecoderInterface<Tp, LzssDummyAuxDecoder<Tp>> {
    std::vector<Tp> tokens = {};

    constexpr float TokenBitSize(Tp token) const {
        return koda::TokenTraits<Tp>::TokenBitSize(token);
    }

    constexpr auto Initialize(koda::BitInputRange auto&& input) {
        return std::forward<decltype(input)>(input);
    }

    constexpr auto Decode(koda::BitInputRange auto&& input,
                          std::ranges::output_range<Tp> auto&& output) {
        auto output_iter = std::ranges::begin(output);
        *output_iter++ = tokens.front();
        tokens.erase(tokens.begin());

        return koda::CoderResult{
            std::move(input), std::ranges::subrange{std::move(output_iter),
                                                    std::ranges::end(output)}};
    }
};

}  // namespace

BeginConstexprTest(LzssEncoder, DecodeTokens) {
    std::vector input_sequence = {
        koda::LzssIntermediateToken<char>{'a'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{0,
                                          1},  // similar to LZ77 in this mode !
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
    std::string expected_result = "ala ma kota a kot ma ale";
    std::string target;

    koda::LzssDecoder<char,
                      LzssDummyAuxDecoder<koda::LzssIntermediateToken<char>>>
        decoder{1024, 4};
}
EndConstexprTest;
