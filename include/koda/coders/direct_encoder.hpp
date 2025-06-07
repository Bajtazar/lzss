#pragma once

#include <koda/coders/token_traits.hpp>

namespace koda {

template <typename Token>
struct IdentityEncoder {
    using Traits = TokenTraits<Token>;

    constexpr explicit IdentityEncoder() noexcept = default;

    constexpr void Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr void Flush(BitOutputRange auto&& output);

    constexpr void operator()(InputRange<Token> auto&& input,
                              BitOutputRange auto&& output);
};

}  // namespace koda

#include <koda/coders/direct_encoder.tpp>
