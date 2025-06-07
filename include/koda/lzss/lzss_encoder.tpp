#pragma once

namespace koda {

template <std::integral InputToken,
          Encoder<LzssIntermediateToken> AuxiliaryEncoder, typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken))
constexpr LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::LzssEncoder(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryEncoder auxiliary_encoder,
    std::optional<size_t> cyclic_buffer_size, const AllocatorTp& allocator)
    : dictionary_and_buffer_{dictionary_size, look_ahead_size,
                             std::move(cyclic_buffer_size), allocator},
      search_tree_{look_ahead_size, allocator},
      auxiliary_encoder_{std::move(auxiliary_encoder)} {}

}  // namespace koda
