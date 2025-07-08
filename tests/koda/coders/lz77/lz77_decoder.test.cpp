#include <koda/coders/lz77/lz77_decoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

static_assert(koda::Decoder<koda::Lz77Decoder<uint8_t>, uint8_t>);

namespace {

template <typename Tp>
class Lz77DummyAuxDecoder
    : public koda::DecoderInterface<Tp, Lz77DummyAuxDecoder<Tp>> {
   public:
    constexpr explicit Lz77DummyAuxDecoder(std::vector<Tp> tokens = {})
        : tokens_{std::move(tokens)} {}

    [[noreturn]] constexpr float TokenBitSize([[maybe_unused]] Tp token) const {
        throw std::runtime_error{"Unused expression!"};
    }

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

using DummyDecoder = Lz77DummyAuxDecoder<koda::Lz77IntermediateToken<char>>;

BeginConstexprTest(Lz77Decoder, DecodeTokens) {
    std::vector input_sequence = {
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
    std::string expected_result = "ala ma kota a kot ma ale";
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::Lz77Decoder<char, DummyDecoder> decoder{
        1024, 4, DummyDecoder{std::move(input_sequence)}};

    decoder(binary_range | koda::views::LittleEndianInput,
            target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result);
}
EndConstexprTest;

BeginConstexprTest(Lz77Decoder, DecodeMoreTokens) {
    std::vector input_sequence = {
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
    std::string expected_result = "std::nullptr_t & nullptr";
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::Lz77Decoder<char, DummyDecoder> decoder{
        1024, 7, DummyDecoder{std::move(input_sequence)}};

    decoder(binary_range | koda::views::LittleEndianInput,
            target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result);
}
EndConstexprTest;

BeginConstexprTest(Lz77Decoder, DecodeTokensRepeatitions) {
    std::vector input_sequence = {
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
    std::string expected_result = "kot kot kot kot kot kot kot";
    std::vector<uint8_t> binary_range = {1};
    std::string target;

    koda::Lz77Decoder<char, DummyDecoder> decoder{
        1024, 3, DummyDecoder{std::move(input_sequence)}};

    decoder(binary_range | koda::views::LittleEndianInput,
            target | koda::views::InsertFromBack);

    ConstexprAssertEqual(target, expected_result);
}
EndConstexprTest;
