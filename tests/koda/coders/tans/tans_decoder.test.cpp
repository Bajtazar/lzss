#include <koda/coders/tans/tans_decoder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

#include <cinttypes>
#include <print>

static_assert(koda::Decoder<koda::TansDecoder<uint8_t, size_t>, uint8_t>);

BeginConstexprTest(TansDecoderTest, UniformDistribution) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2},
                                               {'b', 2},
                                               {'c', 2},
                                               {'d', 2},
                                               {'e', 2},
                                               {'f', 2},
                                               {'g', 2},
                                               {'h', 2}}};
    koda::TansInitTable table{kCounter};
    koda::TansDecoder decoder{table};

    const std::vector<bool> kStream = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                                       0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
                                       0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1,
                                       1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1};
    const std::string kExpected = "abacdaeffagggaah";

    std::string result;

    decoder(kExpected.size(), kStream | std::views::reverse,
            result | koda::views::InsertFromBack);

    ConstexprAssertEqual(result | std::views::reverse, kExpected);
}
EndConstexprTest;

BeginConstexprTest(TansDecoderTest, GeometricDistribution) {
    const koda::Map<char, size_t> kCounter = {{{'a', 8}, {'b', 4}, {'c', 2}}};
    koda::TansInitTable table{kCounter};
    koda::TansDecoder decoder{table};

    const std::vector<bool> kStream = {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1,
                                       0, 0, 0, 1, 1, 1, 0, 1, 1, 1};
    const std::string kExpected = "aabababacaabaaa";

    std::string result;

    decoder(kExpected.size(), kStream | std::views::reverse,
            result | koda::views::InsertFromBack);

    ConstexprAssertEqual(result | std::views::reverse, kExpected);
}
EndConstexprTest;

BeginConstexprTest(TansDecoderTest, UniformDistributionStrided) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2},
                                               {'b', 2},
                                               {'c', 2},
                                               {'d', 2},
                                               {'e', 2},
                                               {'f', 2},
                                               {'g', 2},
                                               {'h', 2}}};

    koda::TansInitTable table{kCounter, 2, 7};
    koda::TansDecoder decoder{table};

    const std::vector<bool> kStream = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1,
                                       0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0,
                                       0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1,
                                       1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1};

    const std::string kExpected = "abacdaeffagggaah";

    std::string result;

    decoder(kExpected.size(), kStream | std::views::reverse,
            result | koda::views::InsertFromBack);

    ConstexprAssertEqual(result | std::views::reverse, kExpected);
}
EndConstexprTest;

BeginConstexprTest(TansDecoderTest, UniformDistributionStridedAndScaled) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2},
                                               {'b', 2},
                                               {'c', 2},
                                               {'d', 2},
                                               {'e', 2},
                                               {'f', 2},
                                               {'g', 2},
                                               {'h', 2}}};

    koda::TansInitTable table{kCounter, 2, 7, 13};
    koda::TansDecoder decoder{table};

    const std::vector<bool> kStream = {1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1,
                                       1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1,
                                       1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
                                       0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0};

    const std::string kExpected = "abacdaeffagggaah";

    std::string result;

    decoder(kExpected.size(), kStream | std::views::reverse,
            result | koda::views::InsertFromBack);

    ConstexprAssertEqual(result | std::views::reverse, kExpected);
}
EndConstexprTest;
