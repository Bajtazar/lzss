#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/coder_traits.hpp>
#include <koda/coders/lzss/lzss_intermediate_token.hpp>
#include <koda/ranges/bit_iterator.hpp>

#include <array>
#include <cinttypes>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
class LzssIntermediateTokenEncoder
    : public EncoderInterface<
          LzssIntermediateTokenEncoder<InputToken, PositionTp, LengthTp>,
          EncoderInterface<LzssIntermediateTokenEncoder<
              InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
              LengthEncoder>>> {
    using TokenTraits = CoderTraits<TokenEncoder>;
    using PositionTraits = CoderTraits<PositionEncoder>;
    using LengthTraits = CoderTraits<LengthEncoder>;

   public:
    using token_type = LzssIntermediateToken<InputToken, PositionTp, LengthTp>;

    static constexpr bool IsAsymetric = TokenTraits::IsAsymetric &&
                                        PositionTraits::IsAsymetric &&
                                        LengthTraits::IsAsymetric;

    constexpr explicit LzssIntermediateTokenEncoder(
        TokenEncoder token_encoder, PositionEncoder position_encoder,
        LengthEncoder length_encoder);

    constexpr float TokenBitSize(const token_type& token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    using Marker = token_type::RepeatitionMarker;

    enum class State : uint8_t {
        kBit = 0,
        kToken = 1,
        kPosition = 2,
        kLength = 3
    };

    TokenEncoder token_encoder_;
    PositionEncoder position_encoder_;
    LengthEncoder length_encoder_;
    State state_ = State::kBit;
    union {
        InputToken symbol[1];
        Marker marker[1];
    } emitter_;

    constexpr void EmitBit(auto& token, auto& output_iter);

    constexpr void EmitToken(auto& output_iter, auto& output_sent);

    static constexpr bool IsSymetric = TokenTraits::IsSymetric &&
                                       PositionTraits::IsSymetric &&
                                       LengthTraits::IsSymetric;

    static_assert(IsSymetric || IsAsymetric,
                  "All of the decoders has to be either symetric or asymetric");
};

}  // namespace koda

#include <koda/coders/lzss/lzss_intermediate_token_encoder.tpp>
