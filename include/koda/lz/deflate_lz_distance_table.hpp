#pragma once

#include <koda/coder.hpp>
#include <koda/utils/concepts.hpp>

#include <cstdlib>

namespace koda {

struct DeflateLzDistanceTable {
    using Token = uint16_t;

    constexpr explicit DeflateLzDistanceTable() = default;

    constexpr void EncodeDistance(auto BitOutputRange output, size_t distance,
                                  size_t match_length,
                                  auto Encoder<Token>& encoder);

    [[nodiscard]] constexpr std::pair<size_t, size_t> DecodeDistance(
        auto BitInputRange input, Token start_token,
        auto Decoder<Token>& decoder);
};

}  // namespace koda
