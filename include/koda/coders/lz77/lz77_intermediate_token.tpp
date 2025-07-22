#pragma once

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
constexpr Lz77IntermediateToken<InputToken, PositionTp, LengthTp>::
    Lz77IntermediateToken(Symbol suffix_symbol, PositionTp match_position,
                          LengthTp match_length) noexcept
    : suffix_symbol_{suffix_symbol},
      match_position_{match_position},
      match_length_{match_length} {}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr Lz77IntermediateToken<InputToken, PositionTp,
                                              LengthTp>::Symbol
Lz77IntermediateToken<InputToken, PositionTp, LengthTp>::suffix_symbol()
    const noexcept {
    return suffix_symbol_;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr PositionTp Lz77IntermediateToken<
    InputToken, PositionTp, LengthTp>::match_position() const noexcept {
    return match_position_;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr LengthTp Lz77IntermediateToken<
    InputToken, PositionTp, LengthTp>::match_length() const noexcept {
    return match_length_;
}

}  // namespace koda

namespace std {

template <typename Token, typename Position, typename Length>
constexpr auto
formatter<koda::Lz77IntermediateToken<Token, Position, Length>>::parse(
    std::format_parse_context& ctx) {
    return ctx.begin();
}

template <typename Token, typename Position, typename Length>
auto formatter<koda::Lz77IntermediateToken<Token, Position, Length>>::format(
    const koda::Lz77IntermediateToken<Token, Position, Length>& obj,
    std::format_context& ctx) const {
    return std::format_to(
        ctx.out(), "Lz77IntermediateToken(symbol={}, position={}, length={})",
        obj.suffix_symbol(), obj.match_position(), obj.match_length());
}

}  // namespace std
