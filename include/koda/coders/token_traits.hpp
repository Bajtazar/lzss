#pragma once

#include <koda/utils/concepts.hpp>
#include <koda/utils/type_dummies.hpp>

#include <cinttypes>
#include <concepts>

namespace koda {

template <typename Traits>
concept TokenTraitsType =
    requires { Traits::TokenType; } &&
    requires(typename Traits::TokenType token, DummyBitInputRange input,
             DummyBitOutputRange output) {
        { Traits::EncodeToken(token, input) } -> std::same_as<void>;
        {
            Traits::DecodeToken(output)
        } -> std::same_as<typename Traits::TokenType>;
    };

template <typename Token>
struct TokenTraits;

template <std::integral Token>
struct TokenTraits<Token> {
    using TokenType = Token;

    static constexpr void EncodeToken(TokenType token,
                                      BitOutputRange auto&& output);

    [[nodiscard]] static constexpr TokenType DecodeToken(
        BitInputRange auto&& input);
};

}  // namespace koda

#include <koda/coders/token_traits.tpp>
