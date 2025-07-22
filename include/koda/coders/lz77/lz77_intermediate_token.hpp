#pragma once

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

}  // namespace koda

template <typename Token, typename Position, typename Length>
struct std::formatter<koda::Lz77IntermediateToken<Token, Position, Length>> {
    constexpr auto parse(std::format_parse_context& ctx);

    auto format(const koda::Lz77IntermediateToken<Token, Position, Length>& obj,
                std::format_context& ctx) const;
};

#include <koda/coders/lz77/lz77_intermediate_token.tpp>
