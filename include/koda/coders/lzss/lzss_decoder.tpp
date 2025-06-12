#pragma once

namespace koda {

template <std::integral Token = uint8_t,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr LzssDecoder<token, AuxiliaryDecoder, Allocator>::LzssDecoder(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryDecoder auxiliary_decoder,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    : dictionary_and_buffer_{FusedDictAndBufferInfo{
          dictionary_size, look_ahead_size, std::move(cyclic_buffer_size),
          std::move(allocator)}},
      decoder_{std::move(auxiliary_decoder)} {}

template <std::integral Token = uint8_t,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr LzssDecoder<token, AuxiliaryDecoder, Allocator>::LzssDecoder(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size = std::nullopt,
    const Allocator& allocator = Allocator{})
    requires std::is_default_constructible_v<AuxiliaryDecoder>
    : LzssDecoder{dictionary_size, look_ahead_size, AuxiliaryDecoder{},
                  std::move(cyclic_buffer_size), std::move(allocator)} {}

template <std::integral Token = uint8_t,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
[[nodiscard]] constexpr auto&&
LzssDecoder<token, AuxiliaryDecoder, Allocator>::auxiliary_decoder(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.decoder_);
}

}  // namespace koda
