#pragma once

#include <koda/coder.hpp>
#include <koda/lz/lz_distance_table.hpp>
#include <koda/utils/concepts.hpp>

#include <memory>

namespace koda {

template <std::integral InputToken = uint8_t,
          std::integral IntermediateToken = uint16_t,
          LzDistanceTable<IntermediateToken> DistanteTable,
          Encoder<IntermediateToken> AuxiliaryEncoder>
    requires(sizeof(InputToken) <= sizeof(IntermediateToken))
class LzssEncoder {
   public:
    constexpr explicit LzssEncoder(DistanteTable distance_table,
                                   AuxiliaryEncoder auxiliary_encoder) noexcept;

    constexpr void operator()(auto InputRange<InputToken>&& input,
                              auto BitOutputRange&& output) const;

   private:
    DistanteTable distance_table_;
    AuxiliaryEncoder auxiliary_encoder_;
};

}  // namespace koda
