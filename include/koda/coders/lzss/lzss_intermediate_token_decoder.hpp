#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/coder_traits.hpp>
#include <koda/coders/lzss/lzss_intermediate_token.hpp>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
class LzssIntermediateTokenDecoder
    : public DecoderInterface<
          LzssIntermediateToken<InputToken, PositionTp, LengthTp>,
          LzssIntermediateTokenDecoder<InputToken, PositionTp, LengthTp,
                                       TokenDecoder, PositionDecoder,
                                       LengthDecoder>> {
    using TokenTraits = CoderTraits<TokenDecoder>;
    using PositionTraits = CoderTraits<PositionDecoder>;
    using LengthTraits = CoderTraits<LengthDecoder>;

   public:
    using token_type = LzssIntermediateToken<InputToken, PositionTp, LengthTp>;

    static constexpr bool IsAsymetric = TokenTraits::IsAsymetric &&
                                        PositionTraits::IsAsymetric &&
                                        LengthTraits::IsAsymetric;

    constexpr explicit LzssIntermediateTokenDecoder(
        TokenDecoder token_decoder, PositionDecoder position_decoder,
        LengthDecoder length_decoder);
    constexpr auto Initialize(BitInputRange auto&& input);

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<token_type> auto&& output);

   private:
    enum class State : uint8_t {
        kBit = 0,
        kToken = 1,
        kPosition = 2,
        kLength = 3
    };

    TokenDecoder token_decoder_;
    PositionDecoder position_decoder_;
    LengthDecoder length_decoder_;
    State state_ = State::kBit;
    PositionTp receiver_position_[0];

    constexpr void ReceiveBit(bool bit);

    constexpr void ReceiveToken(auto& input_iter, auto& input_sent,
                                auto& output_iter);

    constexpr void ReceivePosition(auto& input_iter, auto& input_sent);

    constexpr void ReceiveLength(auto& input_iter, auto& input_sent,
                                 auto& output_iter);

    static constexpr bool IsSymetric = TokenTraits::IsSymetric &&
                                       PositionTraits::IsSymetric &&
                                       LengthTraits::IsSymetric;

    static_assert(IsSymetric || IsAsymetric,
                  "All of the decoders has to be either symetric or asymetric");
};

}  // namespace koda

#include <koda/coders/lzss/lzss_intermediate_token_decoder.tpp>
