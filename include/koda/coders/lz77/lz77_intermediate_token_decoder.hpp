#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/coder_traits.hpp>
#include <koda/coders/lz77/lz77_intermediate_token.hpp>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
class Lz77IntermediateTokenDecoder
    : public DecoderInterface<
          Lz77IntermediateToken<InputToken, PositionTp, LengthTp>,
          DecoderInterface<InputToken, PositionTp, LengthTp, TokenDecoder,
                           PositionDecoder, LengthDecoder>> {
   public:
    constexpr explicit Lz77IntermediateTokenDeoder(
        TokenDecoder token_decoder, PositionDecoder position_decoder,
        LengthDecoder length_decoder);

    constexpr auto Initialize(BitInputRange auto&& input);

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

   private:
    using TokenTraits = CoderTraits<TokenDecoder>;
    using PositionTraits = CoderTraits<PositionDecoder>;
    using LengthTraits = CoderTraits<LengthDecoder>;

    enum class State { kToken, kPosition, kLength };

    TokenDecoder token_decoder_;
    PositionDecoder position_decoder_;
    LengthDecoder length_decoder_;
    State state_;

    static consteval State InitState();
};

}  // namespace koda

#include <koda/coders/lz77/lz77_intermediate_token_decoder.tpp>
