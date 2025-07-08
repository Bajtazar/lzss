#pragma once

#include <koda/coders/token_traits.hpp>
#include <koda/utils/concepts.hpp>

#include <cinttypes>
#include <cstddef>
#include <optional>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp = uint32_t,
          UnsignedIntegral LengthTp = uint16_t>
class [[nodiscard]] Lz77IntermediateToken {
   public:
    using Symbol = InputToken;

    constexpr Lz77IntermediateToken(Symbol suffix_symbol = Symbol{},
                                    PositionTp match_position = 0,
                                    LengthTp match_length = 0) noexcept;

    [[nodiscard]] constexpr Symbol suffix_symbol() const noexcept;

    [[nodiscard]] constexpr PositionTp match_position() const noexcept;

    [[nodiscard]] constexpr LengthTp match_length() const noexcept;

    [[nodiscard]] constexpr std::partial_ordering operator<=>(
        const Lz77IntermediateToken& right) const noexcept;

    [[nodiscard]] constexpr bool operator==(
        const Lz77IntermediateToken& right) const noexcept;

   private:
    Symbol suffix_symbol_;
    PositionTp match_position_;
    LengthTp match_length_;
};

}  // namespace koda
