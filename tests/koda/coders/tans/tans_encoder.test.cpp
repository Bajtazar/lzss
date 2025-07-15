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
