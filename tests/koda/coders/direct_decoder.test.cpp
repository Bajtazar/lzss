#include <koda/coders/direct_decoder.hpp>
#include <koda/coders/direct_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <bitset>
#include <iterator>
#include <vector>

BeginConstexprTest(DirectDecoderTest, DecodeBytes) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    std::vector<uint8_t> target;
    auto in_source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange in_range{koda::LittleEndianOutputBitIter{in_source},
                                   std::default_sentinel};

    koda::DirectEncoder<uint8_t> encoder;

    encoder.Encode(expected, in_range);

    koda::DirectDecoder<uint8_t> decoder;

    koda::LittleEndianInputBitRangeWrapper out_range{target};
    std::vector<uint8_t> reconstruction;
    std::ranges::subrange recon_range{
        koda::BackInserterIterator{reconstruction}, std::default_sentinel};

    decoder.Decode(out_range, recon_range);

    ConstexprAssertEqual(expected, reconstruction);
}
EndConstexprTest(DirectDecoderTest, DecodeBytes);
