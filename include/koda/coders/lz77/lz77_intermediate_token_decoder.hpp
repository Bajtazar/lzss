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
          Lz77IntermediateTokenDecoder<InputToken, PositionTp, LengthTp,
                                       TokenDecoder, PositionDecoder,
                                       LengthDecoder>> {
    using TokenTraits = CoderTraits<TokenDecoder>;
    using PositionTraits = CoderTraits<PositionDecoder>;
    using LengthTraits = CoderTraits<LengthDecoder>;

   public:
    using token_type = Lz77IntermediateToken<InputToken, PositionTp, LengthTp>;

    static constexpr bool IsAsymetric = TokenTraits::IsAsymetric &&
                                        PositionTraits::IsAsymetric &&
                                        LengthTraits::IsAsymetric;

    constexpr explicit Lz77IntermediateTokenDecoder(
        TokenDecoder token_decoder, PositionDecoder position_decoder,
        LengthDecoder length_decoder);

    constexpr auto Initialize(BitInputRange auto&& input);

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<token_type> auto&& output);

   private:
    enum class State : uint8_t { kToken = 0, kPosition = 1, kLength = 2 };

    TokenDecoder token_decoder_;
    PositionDecoder position_decoder_;
    LengthDecoder length_decoder_;
    InputToken token_[1];
    PositionTp position_[1];
    State state_ = State::kToken;

    static constexpr bool IsSymetric = TokenTraits::IsSymetric &&
                                       PositionTraits::IsSymetric &&
                                       LengthTraits::IsSymetric;

    constexpr void ReceiveToken(auto& input_iter, auto& input_sent);

    constexpr void ReceivePosition(auto& input_iter, auto& input_sent);

    constexpr void ReceiveLength(auto& input_iter, auto& input_sent,
                                 auto& output_iter);

    static_assert(IsSymetric || IsAsymetric,
                  "All of the decoders has to be either symetric or asymetric");
};

}  // namespace koda

#include <koda/coders/lz77/lz77_intermediate_token_decoder.tpp>
