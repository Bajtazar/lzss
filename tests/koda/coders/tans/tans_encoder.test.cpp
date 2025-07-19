#include <koda/coders/tans/tans_encoder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <cinttypes>

static_assert(
    koda::SizeAwareEncoder<koda::TansEncoder<uint8_t, size_t>, uint8_t>);

BeginConstexprTest(TansEncoderTest, UniformDistribution) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2},
                                               {'b', 2},
                                               {'c', 2},
                                               {'d', 2},
                                               {'e', 2},
                                               {'f', 2},
                                               {'g', 2},
                                               {'h', 2}}};
    koda::TansInitTable table{kCounter};
    koda::TansEncoder encoder{table};

    const std::vector<bool> kExpected = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                                         0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1,
                                         0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1,
                                         1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1};

    std::string sequence = "abacdaeffagggaah";
    std::vector<bool> stream;

    encoder(sequence, stream | koda::views::InsertFromBack);

    ConstexprAssertEqual(stream, kExpected);
}
EndConstexprTest;

BeginConstexprTest(TansEncoderTest, GeometricDistribution) {
    const koda::Map<char, size_t> kCounter = {{{'a', 10}, {'b', 4}, {'c', 2}}};

    koda::TansInitTable table{kCounter};
    koda::TansEncoder encoder{table};

    const std::vector<bool> kExpected = {0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 1,
                                         0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0};

    std::string sequence = "aabababacaabaaa";
    std::vector<bool> stream;

    encoder(sequence, stream | koda::views::InsertFromBack);

    ConstexprAssertEqual(stream, kExpected);
}
EndConstexprTest;

BeginConstexprTest(TansEncoderTest, UniformDistributionStrided) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2},
                                               {'b', 2},
                                               {'c', 2},
                                               {'d', 2},
                                               {'e', 2},
                                               {'f', 2},
                                               {'g', 2},
                                               {'h', 2}}};

    koda::TansInitTable table{kCounter, 2, 7};
    koda::TansEncoder encoder{table};

    const std::vector<bool> kExpected = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1,
                                         0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0,
                                         0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1,
                                         1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1};

    std::string sequence = "abacdaeffagggaah";
    std::vector<bool> stream;

    encoder(sequence, stream | koda::views::InsertFromBack);

    ConstexprAssertEqual(stream, kExpected);
}
EndConstexprTest;

BeginConstexprTest(TansEncoderTest, UniformDistributionStridedAndScaled) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2},
                                               {'b', 2},
                                               {'c', 2},
                                               {'d', 2},
                                               {'e', 2},
                                               {'f', 2},
                                               {'g', 2},
                                               {'h', 2}}};

    koda::TansInitTable table{kCounter, 2, 7, 64};
    koda::TansEncoder encoder{table};

    const std::vector<bool> kExpected = {
        0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1,
        0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0,
        1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0};

    std::string sequence = "abacdaeffagggaah";
    std::vector<bool> stream;

    encoder(sequence, stream | koda::views::InsertFromBack);

    ConstexprAssertEqual(stream, kExpected);
}
EndConstexprTest;

BeginConstexprTest(TansEncoderTest, DiracDistribution) {
    const koda::Map<char, size_t> kCounter = {{'a', 1}};

    koda::TansInitTable table{kCounter};
    koda::TansEncoder encoder{table};

    std::string sequence = "aaaaaaaaaaaaaaaa";
    std::vector<bool> stream;

    encoder(sequence, stream | koda::views::InsertFromBack);

    ConstexprAssertTrue(stream.empty());
}
EndConstexprTest;

BeginConstexprTest(TansEncoderTest, RandomDistribution) {
    const koda::Map<char, size_t> kCounter = {{{'a', 1},
                                               {'b', 2},
                                               {'c', 5},
                                               {'d', 1},
                                               {'e', 8},
                                               {'f', 7},
                                               {'g', 15},
                                               {'h', 11}}};
    koda::TansInitTable table{kCounter, 0, 1, 64};
    koda::TansEncoder encoder{table};

    const std::vector<bool> kExpected = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1,
        1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1};

    std::string sequence = "abacdaeffagggaah";
    std::vector<bool> stream;

    encoder(sequence, stream | koda::views::InsertFromBack);
    ConstexprAssertEqual(stream, kExpected);
}
EndConstexprTest;
