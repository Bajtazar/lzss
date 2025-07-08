#include <koda/coders/lz77/lz77_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

static_assert(koda::Encoder<koda::Lz77Encoder<uint8_t>, uint8_t>);

namespace {

template <typename Tp>
struct Lz77DummyAuxEncoder
    : public koda::EncoderInterface<Tp, Lz77DummyAuxEncoder<Tp>> {
    constexpr explicit Lz77DummyAuxEncoder() = default;

    std::vector<Tp> tokens = {};

    [[noreturn]] constexpr float TokenBitSize([[maybe_unused]] Tp token) const {
        throw std::runtime_error{"Unused expression!"};
    }

    constexpr auto Encode(koda::InputRange<Tp> auto&& input,
                          koda::BitOutputRange auto&& output) {
        tokens.insert_range(tokens.end(), std::forward<decltype(input)>(input));
        // Circumvent static analysis
        auto iter = std::ranges::begin(input);
        const auto sent = std::ranges::end(input);
        for (; iter != sent; ++iter);
        return koda::CoderResult{iter, sent,
                                 std::forward<decltype(output)>(output)};
    }

    constexpr auto Flush(koda::BitOutputRange auto&& output) {
        return std::forward<decltype(output)>(output);
    }
};

}  // namespace

BeginConstexprTest(Lz77Encoder, EncodeTokens) {
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

    koda::Lz77Encoder<char,
                      Lz77DummyAuxEncoder<koda::Lz77IntermediateToken<char>>>
        encoder{1024, 4};
    encoder(input_sequence, target | koda::views::InsertFromBack |
                                koda::views::LittleEndianOutput);

    ConstexprAssertEqual(encoder.auxiliary_encoder().tokens, expected_result);
    ConstexprAssertTrue(target.empty());
};
EndConstexprTest;
