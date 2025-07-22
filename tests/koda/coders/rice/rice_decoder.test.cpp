#include <koda/coders/coder.hpp>
#include <koda/coders/rice/rice_decoder.hpp>
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

static_assert(koda::Decoder<koda::RiceDecoder<uint8_t>, uint8_t>);

BeginConstexprTest(RiceDecoderTest, DecoderFirstDegree) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13}};
    std::vector<bool> stream;
    std::vector<uint8_t> result;

    koda::RiceEncoder<uint8_t> encoder{1};

    encoder(kInput, stream | koda::views::InsertFromBack);

    koda::RiceDecoder<uint8_t> decoder{1};

    decoder(stream, result | koda::views::InsertFromBack);

    ConstexprAssertEqual(kInput, result);
}
EndConstexprTest;
