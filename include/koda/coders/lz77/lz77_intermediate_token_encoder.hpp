#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/coder_traits.hpp>
#include <koda/coders/lz77/lz77_intermediate_token.hpp>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
class Lz77IntermediateTokenEncoder
    : public EncoderInterface<
          Lz77IntermediateToken<InputToken, PositionTp, LengthTp>,
          EncoderInterface<InputToken, PositionTp, LengthTp, TokenEncoder,
                           PositionEncoder, LengthEncoder>> {
   public:
    constexpr explicit Lz77IntermediateTokenEncoder(
        TokenEncoder token_encoder, PositionEncoder position_encoder,
        LengthEncoder length_encoder);

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    using TokenTraits = CoderTraits<TokenEncoder>;
    using PositionTraits = CoderTraits<PositionEncoder>;
    using LengthTraits = CoderTraits<LengthEncoder>;

    enum class State { kToken, kPosition, kLength };

    TokenEncoder token_encoder_;
    PositionEncoder position_encoder_;
    LengthEncoder length_encoder_;
    State state_ = State::kToken;

    static constexpr bool IsSymetric = TokenTraits::IsSymetrical &&
                                       PositionTraits::IsSymetrical &&
                                       LengthTraits::IsSymetrical;
    static constexpr bool IsAsymetric = TokenTraits::IsAsymetrical &&
                                        PositionTraits::IsAsymetrical &&
                                        LengthTraits::IsAsymetrical;
    static_assert(IsSymetric || IsAsymetric,
                  "All of the decoders has to be either symetric or asymetric");
};

}  // namespace koda

#include <koda/coders/lz77/lz77_intermediate_token_encoder.tpp>
