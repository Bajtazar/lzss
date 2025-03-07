#pragma once

#include <koda/encoder_base.hpp>
#include <koda/lz/lz_distance_table.hpp>

#include <memory>

namespace koda {

class LzssEncoder final : public EncoderBase<uint8_t> {
   public:
    explicit LzssEncoder(std::unique_ptr<LzDistanceTable> distance_table,
                         std::unique_ptr<EncoderBase<uint16_t>>
                             auxiliary_encoder = nullptr) noexcept;

    void operator()(InputTokenStream& input,
                    OutputBinaryStream& output) const override final;

    ~LzssEncoder() noexcept override final = default;

   private:
    std::unique_ptr<LzDistanceTable> distance_table_;
    std::unique_ptr<EncoderBase<uint16_t>> auxiliary_encoder_;
};

}  // namespace koda
