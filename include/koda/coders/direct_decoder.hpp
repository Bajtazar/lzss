#pragma once

#include <koda/coders/token_traits.hpp>

namespace koda {

template <typename Token>
struct DirectDecoder {
    using Traits = TokenTraits<Token>;

    constexpr explicit DirectDecoder() noexcept = default;

    constexpr float TokenBitSize(Token token) const;

    constexpr void Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

    constexpr void operator()(BitInputRange auto&& input,
                              std::ranges::output_range<Token> auto&& output);
};

}  // namespace koda

#include <koda/coders/direct_decoder.tpp>
