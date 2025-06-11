#pragma once

#include <koda/utils/concepts.hpp>
#include <koda/utils/type_dummies.hpp>

#include <cinttypes>
#include <concepts>

namespace koda {

template <BitInputRange Range>
struct TokenTraitsDecodingResult {
    TokenType token;
    Range range;
};

template <typename Traits>
concept TokenTraitsType =
    requires { Traits::TokenType; } &&
    requires(typename Traits::TokenType token, DummyBitInputRange input,
             DummyBitOutputRange output) {
        { Traits::EncodeToken(token, input) } -> BitOutputRange;
        {
            Traits::DecodeToken(output)
        } -> SpecializationOf<TokenTraitsDecodingResult>;
        { Traits::TokenBitSize(token) } -> std::same_as<float>;
    };

template <typename Token>
struct TokenTraits;

template <std::integral Token>
struct TokenTraits<Token> {
    using TokenType = Token;

    static constexpr auto EncodeToken(TokenType token,
                                      BitOutputRange auto&& output);

    [[nodiscard]] static constexpr auto DecodeToken(BitInputRange auto&& input);

    [[nodiscard]] static constexpr float TokenBitSize(TokenType token);
};

}  // namespace koda

#include <koda/coders/token_traits.tpp>
