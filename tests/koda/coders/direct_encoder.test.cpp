#include <koda/coders/coder.hpp>
#include <koda/coders/direct_encoder.hpp>
#include <koda/collections/map.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>

#include <bitset>
#include <iterator>
#include <vector>

static_assert(koda::SizeAwareEncoder<koda::DirectEncoder<uint8_t>, uint8_t>);

BeginConstexprTest(DirectEncoderTest, EncodeBytes) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    std::vector<uint8_t> target;

    koda::DirectEncoder<uint8_t> encoder;

    encoder.Encode(expected, target | koda::views::InsertFromBack |
                                 koda::views::LittleEndianOutput);

    ConstexprAssertEqual(expected, target);
}
EndConstexprTest;

template <typename Tp>
static constexpr std::vector<Tp> ShiftBits(std::vector<Tp> vector,
                                           uint8_t shift) {
    std::vector<Tp> result;
    Tp next = 0;
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
    auto view =
        target | koda::views::InsertFromBack | koda::views::LittleEndianOutput;

    koda::DirectEncoder<uint8_t> encoder;

    auto iter = view.begin();
    *iter++ = 0;
    *iter++ = 0;
    *iter++ = 1;
    *iter++ = 0;

    auto [in_range, out_range] =
        encoder.Encode(start, std::ranges::subrange{iter, view.end()});

    auto iter2 = out_range.begin();
    *iter2++ = 1;
    *iter2++ = 1;
    *iter2++ = 0;
    *iter2++ = 1;

    ConstexprAssertTrue(in_range.empty());
    ConstexprAssertEqual(expected, target);
}
EndConstexprTest;

BeginConstexprTest(DirectEncoderTest, EncodeIntegers) {
    const std::vector<uint32_t> source_range{{0x43'32'12'45, 0x98'32'56'23}};
    const std::vector<uint8_t> expected_range{
        {0x45, 0x12, 0x32, 0x43, 0x23, 0x56, 0x32, 0x98}};

    std::vector<uint8_t> target;

    koda::DirectEncoder<uint32_t> encoder;

    encoder.Encode(source_range, target | koda::views::InsertFromBack |
                                     koda::views::LittleEndianOutput);

    ConstexprAssertEqual(expected_range, target);
}
EndConstexprTest;

BeginConstexprTest(DirectEncoderTest, PartialEncoding) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    std::vector<uint8_t> target;

    koda::DirectEncoder<uint8_t> encoder;

    encoder.EncodeN(
        2, expected,
        target | koda::views::InsertFromBack | koda::views::LittleEndianOutput);

    ConstexprAssertEqual(expected | koda::views::Take(2), target);

    encoder.EncodeN(
        1, expected | std::views::drop(2),
        target | koda::views::InsertFromBack | koda::views::LittleEndianOutput);

    ConstexprAssertEqual(expected | koda::views::Take(3), target);

    encoder(
        expected | std::views::drop(3),
        target | koda::views::InsertFromBack | koda::views::LittleEndianOutput);

    ConstexprAssertEqual(expected, target);
}
EndConstexprTest;
