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
