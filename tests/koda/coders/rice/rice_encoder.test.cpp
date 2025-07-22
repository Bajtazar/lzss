#include <koda/coders/coder.hpp>
#include <koda/coders/rice/rice_encoder.hpp>
#include <koda/collections/map.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>

#include <gtest/gtest.h>

#include <bitset>
#include <iterator>
#include <print>
#include <vector>

static_assert(koda::SizeAwareEncoder<koda::RiceEncoder<uint8_t>, uint8_t>);

BeginConstexprTest(RiceEncoderTest, EncodeFirstDegree) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13}};
    const std::vector<bool> kExpected = {
        1, 1,                   // 1
        0, 0, 1, 0,             // 4
        0, 0, 0, 0, 1, 0,       // 8
        0, 0, 0, 0, 0, 0, 1, 1  // 13
    };
    std::vector<bool> target;

    koda::RiceEncoder<uint8_t> encoder{1};

    encoder(kInput, target | koda::views::InsertFromBack);

    ConstexprAssertEqual(kExpected, target);
}
EndConstexprTest;

// BeginConstexprTest(UniformEncoderTest, PartialInputEncoding) {
//     const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
//     std::vector<uint8_t> target;

//     koda::UniformEncoder<uint8_t> encoder;

//     encoder.EncodeN(
//         2, expected,
//         target | koda::views::InsertFromBack |
//         koda::views::LittleEndianOutput);

//     ConstexprAssertEqual(expected | koda::views::Take(2), target);

//     encoder.EncodeN(
//         1, expected | std::views::drop(2),
//         target | koda::views::InsertFromBack |
//         koda::views::LittleEndianOutput);

//     ConstexprAssertEqual(expected | koda::views::Take(3), target);

//     encoder(
//         expected | std::views::drop(3),
//         target | koda::views::InsertFromBack |
//         koda::views::LittleEndianOutput);

//     ConstexprAssertEqual(expected, target);
// }
// EndConstexprTest;

// BeginConstexprTest(UniformEncoderTest, PartialOutputEncoding) {
//     const std::vector<uint8_t> kSource{{0x43, 0x74, 0x35, 0x33}};
//     auto kExpected = kSource | koda::views::LittleEndianInput |
//                      std::ranges::to<std::vector>();

//     koda::UniformEncoder<uint8_t> encoder;

//     std::vector<bool> stream;

//     auto [in_1, _] = encoder.Encode(
//         kSource, stream | koda::views::InsertFromBack |
//         koda::views::Take(5));

//     ConstexprAssertEqual(stream.size(), 5);
//     ConstexprAssertEqual(stream, kExpected | koda::views::Take(5));

//     auto [in_2, _] =
//         encoder.Encode(std::move(in_1), stream | koda::views::InsertFromBack
//         |
//                                             koda::views::Take(13));

//     ConstexprAssertEqual(stream.size(), 18);
//     ConstexprAssertEqual(stream, kExpected | koda::views::Take(18));

//     encoder(std::move(in_2), stream | koda::views::InsertFromBack);

//     ConstexprAssertEqual(stream.size(), kExpected.size());
//     ConstexprAssertEqual(stream, kExpected);
// }
// EndConstexprTest;
