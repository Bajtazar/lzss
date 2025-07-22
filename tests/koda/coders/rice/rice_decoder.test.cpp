#include <koda/coders/coder.hpp>
#include <koda/coders/rice/rice_decoder.hpp>
#include <koda/coders/rice/rice_encoder.hpp>
#include <koda/collections/map.hpp>
#include <koda/ranges/back_inserter_iterator.hpp>
#include <koda/ranges/bit_iterator.hpp>
#include <koda/tests/tests.hpp>

#include <bitset>
#include <iterator>
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

BeginConstexprTest(RiceDecoderTest, DecoderSecondDegree) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13}};
    std::vector<bool> stream;
    std::vector<uint8_t> result;

    koda::RiceEncoder<uint8_t> encoder{2};

    encoder(kInput, stream | koda::views::InsertFromBack);

    koda::RiceDecoder<uint8_t> decoder{2};

    decoder(stream, result | koda::views::InsertFromBack);

    ConstexprAssertEqual(kInput, result);
}
EndConstexprTest;

BeginConstexprTest(RiceDecoderTest, DecoderFourthDegree) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13, 22, 17, 19}};
    std::vector<bool> stream;
    std::vector<uint8_t> result;

    koda::RiceEncoder<uint8_t> encoder{4};

    encoder(kInput, stream | koda::views::InsertFromBack);

    koda::RiceDecoder<uint8_t> decoder{4};

    decoder(stream, result | koda::views::InsertFromBack);

    ConstexprAssertEqual(kInput, result);
}
EndConstexprTest;

BeginConstexprTest(RiceDecoderTest, PartialInputDecoding) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13, 22, 17, 19}};
    std::vector<bool> stream;
    std::vector<uint8_t> result;

    koda::RiceEncoder<uint8_t> encoder{4};

    encoder(kInput, stream | koda::views::InsertFromBack);

    koda::RiceDecoder<uint8_t> decoder{4};

    std::vector<uint8_t> reconstruction;

    auto [istream_1, _] = decoder.DecodeN(
        2, stream, reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(kInput | koda::views::Take(2), reconstruction);

    auto [istream_2, _] = decoder.DecodeN(
        1, std::move(istream_1), reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(kInput | koda::views::Take(3), reconstruction);

    decoder.Decode(std::move(istream_2),
                   reconstruction | koda::views::InsertFromBack);

    ConstexprAssertEqual(kInput, reconstruction);
}
EndConstexprTest;

BeginConstexprTest(RiceDecoderTest, PartialOutputDecoding) {
    const std::vector<uint8_t> kInput{{1, 4, 8, 13, 22, 17, 19}};
    std::vector<bool> stream;
    std::vector<uint8_t> result;

    koda::RiceEncoder<uint8_t> encoder{4};

    encoder(kInput, stream | koda::views::InsertFromBack);

    koda::RiceDecoder<uint8_t> decoder{4};

    decoder.Decode(stream | koda::views::Take(3),
                   result | koda::views::InsertFromBack);

    ConstexprAssertTrue(result.empty());

    decoder.Decode(stream | std::views::drop(3) | koda::views::Take(4),
                   result | koda::views::InsertFromBack);

    ConstexprAssertEqual(result, kInput | koda::views::Take(1));

    decoder(stream | std::views::drop(7), result | koda::views::InsertFromBack);

    ConstexprAssertEqual(result.size(), kInput.size());
    ConstexprAssertEqual(result, kInput);
}
EndConstexprTest;
