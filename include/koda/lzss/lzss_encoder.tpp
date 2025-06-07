#pragma once

namespace koda {

template <std::integral InputToken,
          Encoder<LzssIntermediateToken> AuxiliaryEncoder, typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken))
constexpr LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::LzssEncoder(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryEncoder auxiliary_encoder,
    std::optional<size_t> cyclic_buffer_size, const AllocatorTp& allocator)
    : dictionary_and_buffer_{FusedDictAndBufferInfo{
          dictionary_size, std::move(cyclic_buffer_size)}},
      search_tree_{look_ahead_size, allocator},
      auxiliary_encoder_{std::move(auxiliary_encoder)} {}

template <std::integral InputToken,
          Encoder<LzssIntermediateToken> AuxiliaryEncoder, typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken))
constexpr LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::LzssEncoder(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size, const AllocatorTp& allocator)
    requires std::is_default_constructible_v<AuxiliaryEncoder>
    : LzssEncoder{dictionary_size, look_ahead_size, AuxiliaryEncoder{},
                  std::move(cyclic_buffer_size), allocator} {}

template <std::integral InputToken,
          Encoder<LzssIntermediateToken> AuxiliaryEncoder, typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken))
constexpr void
LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::operator()(
    InputRange<InputToken> auto&& input, BitOutputRange auto&& output) {
    Encode(std::forward<decltype(input)>(input),
           std::forward<decltype(output)>(output));
    Flush(std::forward<decltype(output)>(output));
}

template <std::integral InputToken,
          Encoder<LzssIntermediateToken> AuxiliaryEncoder, typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken))
constexpr void LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::Flush(
    BitOutputRange auto&& output) {
    auxiliary_encoder_.Flush(std::forward<decltype(output)>(output));
}

}  // namespace koda
