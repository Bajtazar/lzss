#include <koda/coders/tans/tans_encoder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

#include <print>

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
    const koda::Map<char, size_t> kCounter = {{{'a', 8}, {'b', 4}, {'c', 2}}};

    koda::TansInitTable table{kCounter};
    koda::TansEncoder encoder{table};

    const std::vector<bool> kExpected = {0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1,
                                         0, 0, 0, 1, 1, 1, 0, 1, 0, 1};

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

    koda::TansInitTable table{kCounter, 2, 7, 13};
    koda::TansEncoder encoder{table};

    const std::vector<bool> kExpected = {1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1,
                                         1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1,
                                         1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
                                         0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1};

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
