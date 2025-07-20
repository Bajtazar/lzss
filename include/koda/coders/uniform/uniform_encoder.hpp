#pragma once

#include <koda/coders/coder.hpp>

namespace koda {

template <typename Token>
class UniformEncoder : public EncoderInterface<Token, UniformEncoder<Token>> {
   public:
    using token_type = Token;

    using Traits = TokenTraits<Token>;

    constexpr explicit UniformEncoder(uint8_t token_bit_length) noexcept;

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    using BitIter = LittleEndianInputBitIter<const State*>;
    using BitRange = std::pair<BitIter, BitIter>;

    Token token_[0];
    BitRange emitter_;
    uint8_t token_bit_length_;

    constexpr auto EncodeTokens(InputRange<Token> auto&& input, auto iter,
                                auto sentinel);

    constexpr auto FlushEmitter(auto output_iter, const auto& output_sent);

    constexpr void SetEmitter(const Token& token);
};

}  // namespace koda

#include <koda/coders/uniform/uniform_encoder.tpp>
