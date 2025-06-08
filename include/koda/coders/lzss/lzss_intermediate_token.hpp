#pragma once

#include <koda/coders/token_traits.hpp>

#include <cinttypes>
#include <cstddef>
#include <optional>

namespace koda {

template <std::integral InputToken>
class [[nodiscard]] LzssIntermediateToken {
   public:
    using Symbol = InputToken;

    struct RepeatitionMarker {
        uint32_t match_position;
        uint16_t match_length;
    };

    constexpr explicit LzssIntermediateToken(InputToken symbol) noexcept;

    constexpr explicit LzssIntermediateToken(uint32_t match_position,
                                             uint16_t match_length) noexcept;

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

template <std::integral InputToken>
struct TokenTraits<LzssIntermediateToken<InputToken>> {
    using TokenType = LzssIntermediateToken<InputToken>;

    static constexpr void EncodeToken(TokenType token,
                                      BitOutputRange auto&& output);

    [[nodiscard]] static constexpr TokenType DecodeToken(
        BitInputRange auto&& input);

    [[nodiscard]] static constexpr float TokenBitSize(TokenType token);
};

}  // namespace koda

#include <koda/coders/lzss/lzss_intermediate_token.tpp>
