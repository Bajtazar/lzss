#pragma once

#include <koda/coder.hpp>
#include <koda/lz/lz_distance_table.hpp>

#include <concepts>
#include <memory>

namespace koda {

template <typename DistanteTable, Encoder<uint16_t> AuxiliaryEncoder>
class LzssEncoder {
   public:
    explicit LzssEncoder(DistanteTable distance_table,
                         AuxiliaryEncoder auxiliary_encoder) noexcept;

    void operator()(auto std::ranges::input_range&& input,
                    auto BitOutputRange&& output) const;

   private:
    DistanteTable distance_table_;
    AuxiliaryEncoder auxiliary_encoder_;
};

}  // namespace koda
