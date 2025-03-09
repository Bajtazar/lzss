#pragma once

#include <koda/coder.hpp>
#include <koda/utils/concepts.hpp>

#include <memory>
#include <optional>

namespace koda {

class [[nodiscard]] IntermediateToken {
   public:
    struct RepeatitionMarker {
        size_t match_position;
        size_t match_length;
    };

    constexpr explicit IntermediateToken(uint8_t symbol) noexcept;

    constexpr explicit IntermediateToken(size_t match_position,
                                         size_t match_length) noexcept;

    [[nodiscard]] constexpr bool holds_symbol() const noexcept;

    [[nodiscard]] constexpr bool holds_marker() const noexcept;

    [[nodiscard]] constexpr std::optional<uint8_t> get_symbol() const noexcept;

    [[nodiscard]] constexpr std::optional<RepeatitionMarker> get_marker()
        const noexcept;

   private:
    bool holds_distance_match_pair_;
    // We don't need to manually call destructor on an active
    // member since both of them are trivially destructible
    union {
        uint8_t symbol_;
        RepeatitionMarker repeatition_marker_;
    };
};

template <std::integral InputToken = uint8_t,
          Encoder<IntermediateToken> AuxiliaryEncoder>
    requires(sizeof(InputToken) <= sizeof(IntermediateToken))
class LzssEncoder {
   public:
    constexpr explicit LzssEncoder(
        size_t dictionary_size,
        // 1 << (CHAR_BIT * sizeof(InputToken))
        AuxiliaryEncoder auxiliary_encoder = std::nullopt) noexcept;

    constexpr void operator()(InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output) const;

   private:
    size_t dictionary_size_;
    AuxiliaryEncoder auxiliary_encoder_;
};

template <std::integral InputTokenTp, Encoder<IntermediateToken> AuxiliaryEncoderTp>
class LzssEncoderInstance {
public:
    constexpr explicit LzssEncoderInstance(
        size_t dictionary_size,
        AuxiliaryEncoder auxiliary_encoder
    ) : dictionary_size_{dictionary_size},
    auxiliary_encoder_{auxiliary_encoder} {}

private:
    size_t dictionary_size_;
    AuxiliaryEncoder auxiliary_encoder_;
};

template <std::integral InputToken, Encoder<IntermediateToken> AuxiliaryEncoder>
    requires(sizeof(InputToken) <= sizeof(IntermediateToken))
constexpr void LzssEncoder<InputToken, AuxiliaryEncoder>::operator()(
    InputRange<InputToken> auto&& input, BitOutputRange auto&& output) const {

    }

}  // namespace koda

#include <koda/lzss/lzss_encoder.ipp>
