#include <koda/coders/coder.hpp>
#include <koda/coders/direct_decoder.hpp>
#include <koda/coders/direct_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <bitset>
#include <iterator>
#include <vector>

static_assert(koda::Decoder<koda::DirectDecoder<uint8_t>, uint8_t>);

namespace {

constexpr std::vector<uint8_t> ScenarioIEncode(
    const std::vector<uint8_t>& expected) {
    std::vector<uint8_t> target;

    koda::DirectEncoder<uint8_t> encoder;

    encoder.Encode(expected, target | koda::views::InsertFromBack |
                                 koda::views::LittleEndianOutput);

    return target;
}

constexpr std::vector<uint8_t> ScenarioIIEncode(
    const std::vector<uint32_t>& source_range) {
    std::vector<uint8_t> target;

    koda::DirectEncoder<uint32_t> encoder;

    encoder.Encode(source_range, target | koda::views::InsertFromBack |
                                     koda::views::LittleEndianOutput);

    return target;
}

}  // namespace

BeginConstexprTest(DirectDecoderTest, DecodeBytes) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    auto encoded = ScenarioIEncode(expected);

    koda::DirectDecoder<uint8_t> decoder;

    std::vector<uint8_t> reconstruction;

    decoder.Decode(encoded | koda::views::LittleEndianInput,
                   reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(expected, reconstruction);
}
EndConstexprTest;

BeginConstexprTest(DirectDecoderTest, DecodeIntegers) {
    const std::vector<uint32_t> source_range{{0x43'32'12'45, 0x98'32'56'23}};
    auto encoded = ScenarioIIEncode(source_range);

    koda::DirectDecoder<uint32_t> decoder;

    std::vector<uint8_t> reconstruction;

    decoder.Decode(encoded | koda::views::LittleEndianInput,
                   reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(source_range, reconstruction);
}
EndConstexprTest;
