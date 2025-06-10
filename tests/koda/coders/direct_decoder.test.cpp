#include <koda/coders/direct_decoder.hpp>
#include <koda/coders/direct_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <bitset>
#include <iterator>
#include <vector>

namespace {

constexpr std::vector<uint8_t> ScenarioIEncode(
    const std::vector<uint8_t>& expected) {
    std::vector<uint8_t> target;
    auto in_source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange in_range{koda::LittleEndianOutputBitIter{in_source},
                                   std::default_sentinel};

    koda::DirectEncoder<uint8_t> encoder;

    encoder.Encode(expected, in_range);

    return target;
}

constexpr std::vector<uint8_t> ScenarioIIEncode(
    const std::vector<uint32_t>& source_range) {
    std::vector<uint8_t> target;
    auto source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange range{koda::LittleEndianOutputBitIter{source},
                                std::default_sentinel};

    koda::DirectEncoder<uint32_t> encoder;

    encoder.Encode(source_range, range);

    return target;
}

}  // namespace

BeginConstexprTest(DirectDecoderTest, DecodeBytes) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    auto target = ScenarioIEncode(expected);

    koda::DirectDecoder<uint8_t> decoder;

    koda::LittleEndianInputBitRangeWrapper out_range{target};
    std::vector<uint8_t> reconstruction;
    std::ranges::subrange recon_range{
        koda::BackInserterIterator{reconstruction}, std::default_sentinel};

    decoder.Decode(out_range, recon_range);

    ConstexprAssertEqual(expected, reconstruction);
}
EndConstexprTest;

BeginConstexprTest(DirectDecoderTest, DecodeIntegers) {
    const std::vector<uint32_t> source_range{{0x43'32'12'45, 0x98'32'56'23}};

    auto encoded = ScenarioIIEncode(source_range);

    koda::DirectDecoder<uint32_t> decoder;

    koda::LittleEndianInputBitRangeWrapper out_range{encoded};
    std::vector<uint32_t> reconstruction;
    std::ranges::subrange recon_range{
        koda::BackInserterIterator{reconstruction}, std::default_sentinel};

    decoder.Decode(out_range, recon_range);

    ConstexprAssertEqual(source_range, reconstruction);
}
EndConstexprTest;
