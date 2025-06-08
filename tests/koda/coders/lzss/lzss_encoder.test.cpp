#include <koda/coders/lzss/lzss_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

static constexpr size_t kDictionarySize = 1024;
static constexpr size_t kLookAheadSize = 4;

TEST(LzssEncoder, EncodeBytes) {
    std::string input_sequence = "ala ma kota a kot ma ale";
    std::vector<uint8_t> target;
    auto source = koda::MakeLittleEndianOutputSource(
        koda::BackInserterIterator{target}
    );
    std::ranges::subrange output_range{koda::LittleEndianOutputBitIter{source},
                                std::default_sentinel};

    koda::LzssEncoder<char> encoder{1024, 4};
    encoder.Encode(input_sequence, output_range);

    std::cout << std::format("{}\n", target);
};
// EndConstexprTest(LzssEncoder, EncodeBytes);
