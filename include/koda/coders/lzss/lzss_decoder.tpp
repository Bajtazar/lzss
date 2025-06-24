#pragma once

#include <koda/utils/back_inserter_iterator.hpp>

namespace koda {

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr LzssDecoder<Token, AuxiliaryDecoder, Allocator>::LzssDecoder(
    size_t dictionary_size, size_t look_ahead_size,
    AuxiliaryDecoder auxiliary_decoder,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    : dictionary_and_buffer_{FusedDictAndBufferInfo{
          dictionary_size, look_ahead_size, std::move(cyclic_buffer_size),
          std::move(allocator)}},
      auxiliary_decoder_{std::move(auxiliary_decoder)} {}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr LzssDecoder<Token, AuxiliaryDecoder, Allocator>::LzssDecoder(
    size_t dictionary_size, size_t look_ahead_size,
    std::optional<size_t> cyclic_buffer_size, const Allocator& allocator)
    requires std::is_default_constructible_v<AuxiliaryDecoder>
    : LzssDecoder{dictionary_size, look_ahead_size, AuxiliaryDecoder{},
                  std::move(cyclic_buffer_size), std::move(allocator)} {}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssDecoder<Token, AuxiliaryDecoder, Allocator>::Initialize(
    BitInputRange auto&& input) {
    return LoadFusedDict(
        auxiliary_decoder_.Initialize(std::forward<decltype(input)>(input)));
}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
[[nodiscard]] constexpr auto&&
LzssDecoder<Token, AuxiliaryDecoder, Allocator>::auxiliary_decoder(
    this auto&& self) {
    return std::forward_like<decltype(self)>(self.auxiliary_decoder_);
}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssDecoder<Token, AuxiliaryDecoder, Allocator>::LoadFusedDict(
    BitInputRange auto&& input) {
    auto& info = std::get<FusedDictAndBufferInfo>(dictionary_and_buffer_);

    std::vector<IMToken> start_tokens;
    start_tokens.resize(info.look_ahead_size);

    auto [istream, _] = auxiliary_decoder_.DecodeN(
        info.look_ahead_size, std::forward<decltype(input)>(input),
        start_tokens);

    std::vector<Token> tokens{
        std::from_range,
        start_tokens | std::views::transform([](const auto& token) {
            if (auto symbol = token.get_symbol()) {
                return *symbol;
            }
            if consteval {
                throw "Token is not a symbol!";
            } else {
                throw std::runtime_error{"Token is not a symbol!"};
            }
        })};

    dictionary_and_buffer_ = FusedDictionaryAndBuffer<Token>{
        info.dictionary_size, SequenceView{tokens},
        std::move(info.cyclic_buffer_size), std::move(info.allocator)};

    return istream;
}

template <std::integral Token,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder,
          typename Allocator>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
constexpr auto LzssDecoder<Token, AuxiliaryDecoder, Allocator>::Decode(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    return CoderResult{std::move(input), std::move(output)};
}

}  // namespace koda
