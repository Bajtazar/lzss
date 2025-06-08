#pragma once

#include <cinttypes>
#include <cstddef>
#include <optional>

namespace koda {

template <std::integral InputToken>
class [[nodiscard]] LzssIntermediateToken {
   public:
    struct RepeatitionMarker {
        size_t match_position;
        size_t match_length;
        uint16_t position_bitlength;
        uint16_t length_bitlength;
    };

    constexpr explicit LzssIntermediateToken(InputToken symbol) noexcept;

    constexpr explicit LzssIntermediateToken(
        size_t match_position, size_t match_length, uint16_t position_bitlength,
        uint16_t length_bitlength) noexcept;

    [[nodiscard]] constexpr bool holds_symbol() const noexcept;

    [[nodiscard]] constexpr bool holds_marker() const noexcept;

    [[nodiscard]] constexpr std::optional<InputToken> get_symbol()
        const noexcept;

    [[nodiscard]] constexpr std::optional<RepeatitionMarker> get_marker()
        const noexcept;

   private:
    // We don't need to manually call destructor on an active
    // member since both of them are trivially destructible
    union {
        InputToken symbol_;
        RepeatitionMarker repeatition_marker_;
    };
    bool holds_distance_match_pair_;
};

}  // namespace koda

#include <koda/coders/lzss/lzss_intermediate_token.tpp>
