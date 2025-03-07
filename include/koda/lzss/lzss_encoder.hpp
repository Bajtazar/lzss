#pragma once

#include <koda/encoder_base.hpp>

#include <memory>

namespace koda {

class LzssEncoder final : public EncoderBase<uint8_t, bool> {
   public:
    explicit LzssEncoder(std::unique_ptr<EncoderBase<uint16_t, bool>>
                             auxiliary_encoder = nullptr) noexcept;

    void operator()(InputTokenStream& input,
                    OutputTokenStream& output) const override final;

    ~LzssEncoder() noexcept override final = default;

   private:
    std::unique_ptr<EncoderBase<uint16_t, bool>> auxiliary_encoder_;
};

}  // namespace koda
