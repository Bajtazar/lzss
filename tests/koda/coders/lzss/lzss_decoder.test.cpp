#include <koda/coders/lzss/lzss_decoder.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>

namespace {

template <typename Tp>
class LzssDummyAuxDecoder
    : public koda::DecoderInterface<Tp, LzssDummyAuxDecoder<Tp>> {
   public:
    using token_type = Tp;

    constexpr explicit LzssDummyAuxDecoder(std::vector<Tp> tokens = {})
        : tokens_{std::move(tokens)} {}

    constexpr auto Initialize(koda::BitInputRange auto&& input) {
        return koda::AsSubrange(std::forward<decltype(input)>(input));
    }

    constexpr auto Decode(koda::BitInputRange auto&& input,
                          std::ranges::output_range<Tp> auto&& output) {
        auto token_iter = tokens_.begin();
        auto out_iter = std::ranges::begin(output);
        auto out_sent = std::ranges::end(output);
        for (; token_iter != tokens_.end() && out_iter != out_sent;
             ++token_iter, ++out_iter) {
            *out_iter = *token_iter;
        }

        tokens_.erase(tokens_.begin(), token_iter);

        return koda::CoderResult{std::move(input), std::move(out_iter),
                                 std::move(out_sent)};
    }

   private:
    std::vector<Tp> tokens_;
};

}  // namespace

using DummyDecoder = LzssDummyAuxDecoder<koda::LzssIntermediateToken<char>>;

static_assert(koda::Decoder<koda::LzssDecoder<char, DummyDecoder>, char>);

BeginConstexprTest(LzssDecoder, DecodeTokens) {
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
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::LzssDecoder<char, DummyDecoder> decoder{
        1024, 4, DummyDecoder{std::move(input_sequence)}};

    decoder(binary_range | koda::views::LittleEndianInput,
            target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result);
}
EndConstexprTest;

BeginConstexprTest(LzssDecoder, DecodeTokensCritical) {
    std::vector input_sequence = {
        koda::LzssIntermediateToken<char>{'a'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{0, 1},  // ala
        koda::LzssIntermediateToken<char>{0, 8},  // alaalaalaal
    };

    std::string expected_result = "alaalaalaal";
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::LzssDecoder<char, DummyDecoder> decoder{
        1024, 8, DummyDecoder{std::move(input_sequence)}};

    decoder(binary_range | koda::views::LittleEndianInput,
            target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result);
}
EndConstexprTest;

BeginConstexprTest(LzssDecoder, PartialDecoding) {
    std::vector input_sequence = {
        koda::LzssIntermediateToken<char>{'a'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{0, 1},  // ala
        koda::LzssIntermediateToken<char>{0, 8},  // alaalaalaal
    };

    std::string expected_result = "alaalaalaal";
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::LzssDecoder<char, DummyDecoder> decoder{
        1024, 8, DummyDecoder{std::move(input_sequence)}};

    decoder.Initialize(binary_range | koda::views::LittleEndianInput);

    decoder.DecodeN(5, binary_range | koda::views::LittleEndianInput,
                    target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result | koda::views::Take(5));

    decoder.Decode(binary_range | koda::views::LittleEndianInput,
                   target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result);
}
EndConstexprTest;

BeginConstexprTest(LzssDecoder, SplittedPartialDecoding) {
    std::vector input_sequence = {
        koda::LzssIntermediateToken<char>{'a'},
        koda::LzssIntermediateToken<char>{'l'},
        koda::LzssIntermediateToken<char>{0, 1},  // ala
        koda::LzssIntermediateToken<char>{0, 8},  // alaalaalaal
    };

    std::string expected_result = "alaalaalaal";
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::LzssDecoder<char, DummyDecoder> decoder{
        1024, 8, DummyDecoder{std::move(input_sequence)}};

    decoder.Initialize(binary_range | koda::views::LittleEndianInput);

    decoder.DecodeN(5, binary_range | koda::views::LittleEndianInput,
                    target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result | koda::views::Take(5));

    decoder.DecodeN(2, binary_range | koda::views::LittleEndianInput,
                    target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result | koda::views::Take(7));

    decoder.Decode(binary_range | koda::views::LittleEndianInput,
                   target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result);
}
EndConstexprTest;
