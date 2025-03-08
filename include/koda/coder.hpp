#pragma once

#include <koda/utils/type_dummies.hpp>

#include <cinttypes>

namespace koda {

template <typename Tp>
concept Encoder =
    std::integral<Tp> && requires(Tp encoder, DummyInputRange<Tp> input,
                                  DummyBitOutputRange output) {
        { encoder(input, output) } -> std::same_as<void>;
    };

template <typename Tp>
concept Decoder =
    std::integral<Tp> && requires(Tp decoder, DummyBitInputRange input,
                                  DummyOutputRange<Tp> output) {
        { decoder(input, output) } -> std::same_as<void>;
    };

}  // namespace koda
