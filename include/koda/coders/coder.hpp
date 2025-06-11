#pragma once

#include <koda/utils/type_dummies.hpp>

#include <cinttypes>

namespace koda {

template <typename EncoderTp, typename Tp>
concept Encoder = requires(EncoderTp encoder, DummyInputRange<Tp> input,
                           DummyBitOutputRange output) {
    encoder.Encode(input, output);
    encoder.Flush(output);
    encoder(input, output);
};

template <typename EncoderTp, typename Tp>
concept SizeAwareEncoder =
    Encoder<EncoderTp, Tp> && requires(EncoderTp encoder, Tp token) {
        { encoder.TokenBitSize(token) } -> std::same_as<float>;
    };

template <typename TokenTp, BitInputRange InputRangeTp,
          std::ranges::output_range<Tp> OutputRangeTp>
struct DecodingResult {
    InputRangeTp input_range;
    OutputRangeTp output_range;
};

template <typename DecoderTp, typename Tp>
concept Decoder = requires(DecoderTp decoder, DummyBitInputRange input,
                           DummyOutputRange<Tp> output) {
    { decoder.Decode(input, output) } -> SpecializationOf<DecodingResult>;
    { decoder(input, output) } -> SpecializationOf<DecodingResult>;
};

template <typename DecoderTp, typename Tp>
concept SizeAwareDecoder =
    Decoder<DecoderTp, Tp> && requires(DecoderTp decoder, Tp token) {
        { decoder.TokenBitSize(token) } -> std::same_as<float>;
    };

}  // namespace koda
