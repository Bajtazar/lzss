#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/token_traits.hpp>

namespace koda {

template <typename Token>
struct DirectDecoder : public DecoderInterface<Token, DirectDecoder<Token>> {
    using Traits = TokenTraits<Token>;

    constexpr explicit DirectDecoder() noexcept = default;

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);
};

}  // namespace koda

#include <koda/coders/direct_decoder.tpp>
