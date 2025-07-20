#pragma once

#include <koda/coders/coder.hpp>

#include <array>
#include <concepts>

namespace koda {

template <std::integral Token>
class UniformEncoder : public EncoderInterface<Token, UniformEncoder<Token>> {
   public:
    using token_type = Token;

    constexpr explicit UniformEncoder(
        uint8_t token_bit_size = sizeof(Token) * CHAR_BIT) noexcept;

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    using BitIter = LittleEndianInputBitIter<const Token*>;
    using BitRange = std::pair<BitIter, BitIter>;

    Token token_[1];
    BitRange emitter_;
    uint8_t token_bit_size_;

    constexpr auto EncodeTokens(InputRange<Token> auto&& input, auto iter,
                                auto sentinel);

    constexpr auto FlushEmitter(auto output_iter, const auto& output_sent);

    constexpr void SetEmitter(const Token& token);
};

}  // namespace koda

#include <koda/coders/uniform/uniform_encoder.tpp>
