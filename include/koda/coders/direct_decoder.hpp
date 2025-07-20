#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/token_traits.hpp>

namespace koda {

template <typename Token>
struct DirectDecoder : public DecoderInterface<Token, DirectDecoder<Token>> {
    using token_type = Token;

    using Traits = TokenTraits<Token>;

    constexpr explicit DirectDecoder() noexcept = default;

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

    constexpr auto Initialize(BitInputRange auto&& input);
};

}  // namespace koda

#include <koda/coders/direct_decoder.tpp>
