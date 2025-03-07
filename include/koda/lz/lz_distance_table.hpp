#pragma once

#include <koda/decoder_base.hpp>
#include <koda/encoder_base.hpp>

#include <cstdlib>
#include <istream>
#include <ostream>

namespace koda {

class LzDistanceTable {
   public:
    using InputBinaryStream = std::basic_istream<bool>;
    using OutputBinaryStream = std::basic_ostream<bool>;

    struct [[nodiscard]] RepeatitionPosition {
        size_t distance;
        size_t match_length;
    };

    constexpr explicit LzDistanceTable() noexcept = default;

    virtual void EncodeDistance(
        OutputBinaryStream& stream, size_t distance, size_t match_length,
        const EncoderBase<uint16_t>& auxiliary_encoder) const = 0;

    virtual void DecodeDistance(
        InputBinaryStream& stream, uint16_t start_token,
        const DecoderBase<uint16_t>& auxiliary_decoder) const = 0;

    virtual ~LzDistanceTable() noexcept = default;

   protected:
    constexpr LzDistanceTable(const LzDistanceTable&) noexcept = default;
    constexpr LzDistanceTable(LzDistanceTable&&) noexcept = default;

    constexpr LzDistanceTable& operator=(const LzDistanceTable&) noexcept =
        default;
    constexpr LzDistanceTable& operator=(LzDistanceTable&&) noexcept = default;
};

}  // namespace koda
