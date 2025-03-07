#pragma once

#include <koda/decoder_base.hpp>
#include <koda/encoder_base.hpp>

#include <cstdlib>
#include <istream>
#include <ostream>

namespace koda {

class LzssDistanceTable {
   public:
    using InputBinaryStream = std::basic_istream<bool>;
    using OutputBinaryStream = std::basic_ostream<bool>;

    struct [[nodiscard]] RepeatitionPosition {
        size_t distance;
        size_t match_length;
    };

    constexpr explicit LzssDistanceTable() noexcept = default;

    virtual void EncodeDistance(
        OutputBinaryStream& stream, size_t distance, size_t match_length,
        const EncoderBase<uint16_t>& auxiliary_encoder) const = 0;

    virtual void DecodeDistance(
        InputBinaryStream& stream, uint16_t start_token,
        const DecoderBase<uint16_t>& auxiliary_decoder) const = 0;

    virtual ~LzssDistanceTable() noexcept = default;

   protected:
    constexpr LzssDistanceTable(const LzssDistanceTable&) noexcept = default;
    constexpr LzssDistanceTable(LzssDistanceTable&&) noexcept = default;

    constexpr LzssDistanceTable& operator=(const LzssDistanceTable&) noexcept =
        default;
    constexpr LzssDistanceTable& operator=(LzssDistanceTable&&) noexcept =
        default;
};

}  // namespace koda
