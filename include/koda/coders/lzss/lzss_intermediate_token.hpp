#pragma once

#include <koda/utils/concepts.hpp>

#include <cinttypes>
#include <cstddef>
#include <format>
#include <optional>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp = uint32_t,
          UnsignedIntegral LengthTp = uint16_t>
class [[nodiscard]] LzssIntermediateToken {
   public:
    using Symbol = InputToken;

    struct RepeatitionMarker {
        PositionTp match_position;
        LengthTp match_length;

        [[nodiscard]] constexpr auto operator<=>(
            const RepeatitionMarker& right) const noexcept = default;

        [[nodiscard]] constexpr bool operator==(
            const RepeatitionMarker& right) const noexcept = default;
    };

    constexpr explicit LzssIntermediateToken(
        InputToken symbol = InputToken{}) noexcept;

    constexpr explicit LzssIntermediateToken(PositionTp match_position,
                                             LengthTp match_length) noexcept;

    constexpr LzssIntermediateToken(
        const LzssIntermediateToken& other) noexcept;

    constexpr LzssIntermediateToken& operator=(
        const LzssIntermediateToken& other) noexcept;

    [[nodiscard]] constexpr bool holds_symbol() const noexcept;

    [[nodiscard]] constexpr bool holds_marker() const noexcept;

    [[nodiscard]] constexpr std::optional<InputToken> get_symbol()
        const noexcept;

    [[nodiscard]] constexpr std::optional<RepeatitionMarker> get_marker()
        const noexcept;

    [[nodiscard]] constexpr std::partial_ordering operator<=>(
        const LzssIntermediateToken& right) const noexcept;

    [[nodiscard]] constexpr bool operator==(
        const LzssIntermediateToken& right) const noexcept;

    constexpr ~LzssIntermediateToken();

   private:
    // We don't need to manually call destructor on an active
    // member since both of them are trivially destructible
    union {
        InputToken symbol_;
        RepeatitionMarker repeatition_marker_;
    };
    bool holds_distance_match_pair_;

    constexpr void Destroy();
};

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
struct TokenTraits<LzssIntermediateToken<InputToken, PositionTp, LengthTp>> {
    using TokenType = LzssIntermediateToken<InputToken, PositionTp, LengthTp>;

    static constexpr auto EncodeToken(TokenType token,
                                      BitOutputRange auto&& output);

    [[nodiscard]] static constexpr auto DecodeToken(BitInputRange auto&& input);

    [[nodiscard]] static constexpr float TokenBitSize(TokenType token);
};

}  // namespace koda

template <typename Token, typename Position, typename Length>
struct std::formatter<koda::LzssIntermediateToken<Token, Position, Length>> {
    constexpr auto parse(std::format_parse_context& ctx);

    auto format(const koda::LzssIntermediateToken<Token, Position, Length>& obj,
                std::format_context& ctx) const;
};

#include <koda/coders/lzss/lzss_intermediate_token.tpp>
