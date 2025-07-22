#pragma once

#include <koda/coders/coder.hpp>

namespace koda {

template <std::integral Token>
class RiceEncoder : public EncoderInterface<Token, RiceEncoder> {
   public:
    using token_type = Token;

    constexpr explicit RiceEncoder(uint8_t order);

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    Token mask_;
    Token limit_;
    Token token_{};
    uint8_t order_;
    uint8_t bits_ = 0;

    constexpr auto SetEmitter(const Token& token, auto& iter);

    constexpr auto& FlushEmitter(auto& iter, const auto& sentinel);

    constexpr auto EncodeTokens(InputRange<Token> auto&& input, auto iter,
                                auto sentinel);
};

}  // namespace koda

#include <koda/coders/rice/rice_encoder.tpp>
