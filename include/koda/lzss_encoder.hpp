#pragma once

#include <koda/encoder_base.hpp>

#include <cinttypes>

namespace koda {

class LzssEncoder final : public EncoderBase<uint8_t, uint16_t> {
   public:
    explicit LzssEncoder();

    void operator()(InputTokenStream& input,
                    OutputTokenStream& output) const override final;

    ~LzssEncoder() noexcept override final = default;
};

}  // namespace koda
