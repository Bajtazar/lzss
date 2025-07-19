#pragma once

#include <koda/coders/token_traits.hpp>
#include <koda/utils/concepts.hpp>

#include <cinttypes>
#include <cstddef>
#include <format>
#include <optional>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp = uint32_t,
          UnsignedIntegral LengthTp = uint16_t>
class [[nodiscard]] Lz77IntermediateToken {
   public:
    using Symbol = InputToken;
    using Position = PositionTp;
    using Length = LengthTp;

    constexpr Lz77IntermediateToken(Symbol suffix_symbol = Symbol{},
                                    PositionTp match_position = 0,
                                    LengthTp match_length = 0) noexcept;

    [[nodiscard]] constexpr Symbol suffix_symbol() const noexcept;

    [[nodiscard]] constexpr PositionTp match_position() const noexcept;

    [[nodiscard]] constexpr LengthTp match_length() const noexcept;

    [[nodiscard]] constexpr std::weak_ordering operator<=>(
        const Lz77IntermediateToken& right) const noexcept = default;

    [[nodiscard]] constexpr bool operator==(
        const Lz77IntermediateToken& right) const noexcept = default;

   private:
    Symbol suffix_symbol_;
    PositionTp match_position_;
    LengthTp match_length_;
};

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
struct TokenTraits<Lz77IntermediateToken<InputToken, PositionTp, LengthTp>> {
    using TokenType = Lz77IntermediateToken<InputToken, PositionTp, LengthTp>;

    static constexpr auto EncodeToken(TokenType token,
                                      BitOutputRange auto&& output);

    [[nodiscard]] static constexpr auto DecodeToken(BitInputRange auto&& input);

    [[nodiscard]] static constexpr float TokenBitSize(TokenType token);
};

}  // namespace koda

template <typename Token>
struct std::formatter<koda::Lz77IntermediateToken<Token>> {
    constexpr auto parse(std::format_parse_context& ctx);

    auto format(const koda::Lz77IntermediateToken<Token>& obj,
                std::format_context& ctx) const;
};

#include <koda/coders/lz77/lz77_intermediate_token.tpp>
