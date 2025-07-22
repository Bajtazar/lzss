#pragma once

#include <koda/coders/coder.hpp>
#include <koda/utils/concepts.hpp>

namespace koda {

template <UnsignedIntegral Token>
class RiceEncoder : public EncoderInterface<Token, RiceEncoder<Token>> {
   public:
    using token_type = Token;

    constexpr explicit RiceEncoder(size_t order);

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    Token mask_;
    Token token_{};
    size_t limit_;
    size_t order_;
    size_t bits_ = 0;

    constexpr auto SetEmitter(const Token& token, auto iter);

    constexpr auto FlushEmitter(auto iter, const auto& sentinel);

    constexpr auto EncodeTokens(InputRange<Token> auto&& input, auto iter,
                                auto sentinel);
};

}  // namespace koda

#include <koda/coders/rice/rice_encoder.tpp>
