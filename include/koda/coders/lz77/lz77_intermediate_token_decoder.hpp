#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/coder_traits.hpp>
#include <koda/coders/lz77/lz77_intermediate_token.hpp>

#include <array>
#include <cinttypes>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
class Lz77IntermediateTokenDecoder
    : public DecoderInterface<
          Lz77IntermediateToken<InputToken, PositionTp, LengthTp>,
          DecoderInterface<InputToken, PositionTp, LengthTp, TokenDecoder,
                           PositionDecoder, LengthDecoder>> {
    using TokenTraits = CoderTraits<TokenDecoder>;
    using PositionTraits = CoderTraits<PositionDecoder>;
    using LengthTraits = CoderTraits<LengthDecoder>;

   public:
    static constexpr bool IsAsymetric = TokenTraits::IsAsymetric &&
                                        PositionTraits::IsAsymetric &&
                                        LengthTraits::IsAsymetric;

    constexpr explicit Lz77IntermediateTokenDeoder(
        TokenDecoder token_decoder, PositionDecoder position_decoder,
        LengthDecoder length_decoder);

    constexpr auto Initialize(BitInputRange auto&& input);

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

   private:
    enum class State : uint8_t { kToken = 0, kPosition = 1, kLength = 2 };

    TokenDecoder token_decoder_;
    PositionDecoder position_decoder_;
    LengthDecoder length_decoder_;
    State state_ = IsSymetric ? kToken : kLength;

    static constexpr bool IsSymetric = TokenTraits::IsSymetric &&
                                       PositionTraits::IsSymetric &&
                                       LengthTraits::IsSymetric;

    static constexpr std::array<State, 3> kNextState{
        IsSymetric ? {kPosition, kLength, kToken}
                   : {kPosition, kToken, kLength}};

    static_assert(IsSymetric || IsAsymetric,
                  "All of the decoders has to be either symetric or asymetric");
};

}  // namespace koda

#include <koda/coders/lz77/lz77_intermediate_token_decoder.tpp>
