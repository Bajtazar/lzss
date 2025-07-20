#include <koda/coders/coder.hpp>
#include <koda/coders/uniform/uniform_decoder.hpp>
#include <koda/coders/uniform/uniform_encoder.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>

#include <bitset>
#include <iterator>
#include <vector>

static_assert(koda::Decoder<koda::UniformDecoder<uint8_t>, uint8_t>);

namespace {

template <typename Tp>
constexpr std::vector<uint8_t> Encode(const std::vector<Tp>& expected) {
    std::vector<uint8_t> target;

    koda::UniformEncoder<Tp> encoder;

    encoder.Encode(expected, target | koda::views::InsertFromBack |
                                 koda::views::LittleEndianOutput);

    return target;
}

}  // namespace

BeginConstexprTest(UniformDecoderTest, DecodeBytes) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    auto encoded = Encode(expected);

    koda::UniformDecoder<uint8_t> decoder;

    std::vector<uint8_t> reconstruction;

    decoder.Decode(encoded | koda::views::LittleEndianInput,
                   reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(expected, reconstruction);
}
EndConstexprTest;

BeginConstexprTest(UniformDecoderTest, DecodeIntegers) {
    const std::vector<uint32_t> expected{{0x43'32'12'45, 0x98'32'56'23}};
    auto encoded = Encode(expected);

    koda::UniformDecoder<uint32_t> decoder;

    std::vector<uint32_t> reconstruction;

    decoder.Decode(encoded | koda::views::LittleEndianInput,
                   reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(expected, reconstruction);
}
EndConstexprTest;

BeginConstexprTest(UniformDecoderTest, PartialInputDecoding) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    auto encoded = Encode(expected);

    koda::UniformDecoder<uint8_t> decoder;

    std::vector<uint8_t> reconstruction;

    auto [istream_1, _] =
        decoder.DecodeN(2, encoded | koda::views::LittleEndianInput,
                        reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(expected | koda::views::Take(2), reconstruction);

    auto [istream_2, _] = decoder.DecodeN(
        1, std::move(istream_1), reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(expected | koda::views::Take(3), reconstruction);

    decoder.Decode(std::move(istream_2),
                   reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(expected, reconstruction);
}
EndConstexprTest;

BeginConstexprTest(UniformDecoderTest, PartialOutputDecoding) {
    const std::vector<uint8_t> kExpected{{0x43, 0x74, 0x35, 0x33}};
    auto stream = Encode(kExpected);

    koda::UniformDecoder<uint8_t> decoder;

    std::vector<uint8_t> result;

    decoder.Decode(
        stream | koda::views::LittleEndianInput | koda::views::Take(5),
        result | koda::views::InsertFromBack);

    ConstexprAssertTrue(result.empty());

    decoder.Decode(stream | koda::views::LittleEndianInput |
                       std::views::drop(5) | koda::views::Take(5),
                   result | koda::views::InsertFromBack);

    ConstexprAssertEqual(result, kExpected | koda::views::Take(1));

    decoder(stream | koda::views::LittleEndianInput | std::views::drop(10),
            result | koda::views::InsertFromBack);

    ConstexprAssertEqual(result.size(), kExpected.size());
    ConstexprAssertEqual(result, kExpected);
}
EndConstexprTest;
