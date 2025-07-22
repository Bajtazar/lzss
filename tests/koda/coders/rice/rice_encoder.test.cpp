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

BeginConstexprTest(RiceEncoderTest, EncodeSecondDegree) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13}};
    const std::vector<bool> kExpected = {
        1, 0, 1,          // 1
        0, 1, 0, 0,       // 4
        0, 0, 1, 0, 0,    // 8
        0, 0, 0, 1, 0, 1  // 13
    };
    std::vector<bool> target;

    koda::RiceEncoder<uint8_t> encoder{2};

    encoder(kInput, target | koda::views::InsertFromBack);

    ConstexprAssertEqual(kExpected, target);
}
EndConstexprTest;

BeginConstexprTest(RiceEncoderTest, EncodeFourthDegree) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13, 16}};
    const std::vector<bool> kExpected = {
        1, 0, 0, 0, 1,    // 1
        1, 0, 1, 0, 0,    // 4
        1, 1, 0, 0, 0,    // 8
        1, 1, 1, 0, 1,    // 13
        0, 1, 0, 0, 0, 0  // 16
    };
    std::vector<bool> target;

    koda::RiceEncoder<uint8_t> encoder{4};

    encoder(kInput, target | koda::views::InsertFromBack);

    ConstexprAssertEqual(kExpected, target);
}
EndConstexprTest;

BeginConstexprTest(RiceEncoderTest, PartialInputEncoding) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13, 16}};
    const std::vector<bool> kExpected = {
        1, 0, 0, 0, 1,    // 1
        1, 0, 1, 0, 0,    // 4
        1, 1, 0, 0, 0,    // 8
        1, 1, 1, 0, 1,    // 13
        0, 1, 0, 0, 0, 0  // 16
    };
    std::vector<bool> target;

    koda::RiceEncoder<uint8_t> encoder{4};

    encoder.EncodeN(2, kInput, target | koda::views::InsertFromBack);

    ConstexprAssertEqual(kExpected | koda::views::Take(10), target);

    encoder.EncodeN(1, kInput | std::views::drop(2),
                    target | koda::views::InsertFromBack);

    ConstexprAssertEqual(kExpected | koda::views::Take(15), target);

    encoder(kInput | std::views::drop(3), target | koda::views::InsertFromBack);

    ConstexprAssertEqual(kExpected, target);
}
EndConstexprTest;

BeginConstexprTest(RiceEncoderTest, PartialOutputEncoding) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13, 16}};
    const std::vector<bool> kExpected = {
        1, 0, 0, 0, 1,    // 1
        1, 0, 1, 0, 0,    // 4
        1, 1, 0, 0, 0,    // 8
        1, 1, 1, 0, 1,    // 13
        0, 1, 0, 0, 0, 0  // 16
    };
    std::vector<bool> stream;

    koda::RiceEncoder<uint8_t> encoder{4};

    auto [in_1, _] = encoder.Encode(
        kInput, stream | koda::views::InsertFromBack | koda::views::Take(5));

    ConstexprAssertEqual(stream.size(), 5);
    ConstexprAssertEqual(stream, kExpected | koda::views::Take(5));

    auto [in_2, _] =
        encoder.Encode(std::move(in_1), stream | koda::views::InsertFromBack |
                                            koda::views::Take(13));

    ConstexprAssertEqual(stream.size(), 18);
    ConstexprAssertEqual(stream, kExpected | koda::views::Take(18));

    encoder(std::move(in_2), stream | koda::views::InsertFromBack);

    ConstexprAssertEqual(stream.size(), kExpected.size());
    ConstexprAssertEqual(stream, kExpected);
}
EndConstexprTest;
