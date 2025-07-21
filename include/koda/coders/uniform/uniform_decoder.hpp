#pragma once

#include <koda/coders/coder.hpp>

namespace koda {

template <std::integral Token>
class UniformDecoder : public DecoderInterface<Token, UniformDecoder<Token>> {
   public:
    using token_type = Token;

    constexpr explicit UniformDecoder(
        size_t token_bit_size = sizeof(Token) * CHAR_BIT) noexcept;

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

    constexpr auto Initialize(BitInputRange auto&& input);

   private:
    using BitIter = LittleEndianOutputBitIter<Token*>;
    using BitRange = std::pair<BitIter, BitIter>;

    Token token_[1]{};
    BitRange receiver_;
    size_t token_bit_size_;

    constexpr auto SetReceiver(auto iter, const auto& sent);

    constexpr Token DecodeToken();
};

}  // namespace koda

#include <koda/coders/uniform/uniform_decoder.tpp>
