#include <koda/coders/direct_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <bitset>
#include <iterator>
#include <vector>

BeginConstexprTest(DirectEncoderTest, EncodeBytes) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    std::vector<uint8_t> target;
    auto source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange range{koda::LittleEndianOutputBitIter{source},
                                std::default_sentinel};

    koda::DirectEncoder<uint8_t> encoder;

    encoder.Encode(expected, range);

    ConstexprAssertEqual(expected, target);
}
EndConstexprTest(DirectEncoderTest, EncodeBytes);

static constexpr std::vector<uint8_t> ShiftBits(std::vector<uint8_t> vector,
                                                uint8_t shift) {
    std::vector<uint8_t> result;
    uint8_t next = 0;
    for (auto element : vector) {
        result.push_back((element << shift) | next);
        next = element >> shift;
    }
    result.push_back(next);
    return result;
}

BeginConstexprTest(DirectEncoderTest, EncodeBytesWithStride) {
    const std::vector<uint8_t> start{
        {0x43, 0x74, 0x35, 0x33, 0x54, 0x01, 0x84}};
    std::vector<uint8_t> expected = ShiftBits(start, 4);
    expected.front() |= 0b100;
    expected.back() |= 0b10110000;

    std::vector<uint8_t> target;
    auto source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{target});
    std::ranges::subrange range{koda::LittleEndianOutputBitIter{source},
                                std::default_sentinel};

    koda::DirectEncoder<uint8_t> encoder;

    auto iter = range.begin();
    *iter++ = 0;
    *iter++ = 0;
    *iter++ = 1;
    *iter++ = 0;

    encoder.Encode(start, range);

    auto iter2 = range.begin();
    *iter2++ = 1;
    *iter2++ = 1;
    *iter2++ = 0;
    *iter2++ = 1;

    ConstexprAssertEqual(expected, target);
}
EndConstexprTest(DirectEncoderTest, EncodeBytes);
