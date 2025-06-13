#include <koda/coders/coder.hpp>
#include <koda/coders/direct_decoder.hpp>
#include <koda/coders/direct_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <bitset>
#include <iterator>
#include <vector>

static_assert(koda::SizeAwareDecoder<koda::DirectDecoder<uint8_t>, uint8_t>);

namespace {

template <typename Tp>
constexpr std::vector<uint8_t> Encode(const std::vector<Tp>& expected) {
    std::vector<uint8_t> target;

    koda::DirectEncoder<Tp> encoder;

    encoder.Encode(expected, target | koda::views::InsertFromBack |
                                 koda::views::LittleEndianOutput);

    return target;
}

}  // namespace

BeginConstexprTest(DirectDecoderTest, DecodeBytes) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    auto encoded = Encode(expected);

    koda::DirectDecoder<uint8_t> decoder;

    std::vector<uint8_t> reconstruction;

    decoder.Decode(encoded | koda::views::LittleEndianInput,
                   reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(expected, reconstruction);
}
EndConstexprTest;

BeginConstexprTest(DirectDecoderTest, DecodeIntegers) {
    const std::vector<uint32_t> expected{{0x43'32'12'45, 0x98'32'56'23}};
    auto encoded = Encode(expected);

    koda::DirectDecoder<uint32_t> decoder;

    std::vector<uint32_t> reconstruction;

    decoder.Decode(encoded | koda::views::LittleEndianInput,
                   reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(expected, reconstruction);
}
EndConstexprTest;
