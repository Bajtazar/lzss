#pragma once

#include <koda/utils/type_dummies.hpp>

#include <cinttypes>

namespace koda {

template <typename EncoderTp, typename Tp>
concept Encoder =
    std::integral<Tp> &&
    requires(EncoderTp encoder, DummyInputRange<Tp> input,
             DummyBitOutputRange output) { encoder(input, output); };

template <typename DecoderTp, typename Tp>
concept Decoder =
    std::integral<Tp> &&
    requires(DecoderTp decoder, DummyBitInputRange input,
             DummyOutputRange<Tp> output) { decoder(input, output); };

}  // namespace koda
