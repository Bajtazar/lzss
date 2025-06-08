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

template <typename DecoderTp, typename Tp>
concept Decoder = requires(DecoderTp decoder, DummyBitInputRange input,
                           DummyOutputRange<Tp> output) {
    decoder.Initialize(input);
    decoder.Decode(input, output);
    decoder(input, output);
};

}  // namespace koda
