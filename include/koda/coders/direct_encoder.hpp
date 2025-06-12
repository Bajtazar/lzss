#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/token_traits.hpp>

namespace koda {

template <typename Token>
struct DirectEncoder : public EncoderInterface<Token, DirectEncoder<Token>> {
    using Traits = TokenTraits<Token>;

    constexpr explicit DirectEncoder() noexcept = default;

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);
};

}  // namespace koda

#include <koda/coders/direct_encoder.tpp>
