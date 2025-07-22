#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/coder_traits.hpp>
#include <koda/coders/lz77/lz77_intermediate_token.hpp>

#include <array>
#include <cinttypes>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
class Lz77IntermediateTokenEncoder
    : public EncoderInterface<
          Lz77IntermediateToken<InputToken, PositionTp, LengthTp>,
          Lz77IntermediateTokenEncoder<InputToken, PositionTp, LengthTp,
                                       TokenEncoder, PositionEncoder,
                                       LengthEncoder>> {
    using TokenTraits = CoderTraits<TokenEncoder>;
    using PositionTraits = CoderTraits<PositionEncoder>;
    using LengthTraits = CoderTraits<LengthEncoder>;

   public:
    using token_type = Lz77IntermediateToken<InputToken, PositionTp, LengthTp>;

    static constexpr bool IsAsymetric = TokenTraits::IsAsymetric &&
                                        PositionTraits::IsAsymetric &&
                                        LengthTraits::IsAsymetric;

    constexpr explicit Lz77IntermediateTokenEncoder(
        TokenEncoder token_encoder, PositionEncoder position_encoder,
        LengthEncoder length_encoder);

    constexpr float TokenBitSize(const token_type& token) const;

    constexpr auto Encode(InputRange<token_type> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    enum class State : uint8_t {
        kToken = 0,
        kPosition = 1,
        kLength = 2,
        kInsert = 3
    };

    TokenEncoder token_encoder_;
    PositionEncoder position_encoder_;
    LengthEncoder length_encoder_;
    struct {
        InputToken symbol_[1];
        PositionTp position_[1];
        LengthTp length_[1];
    };
    State state_ = State::kInsert;

    static constexpr bool IsSymetric = TokenTraits::IsSymetric &&
                                       PositionTraits::IsSymetric &&
                                       LengthTraits::IsSymetric;

    constexpr void InsertSymbol(const token_type& token);

    constexpr void EmitToken(auto& output_iter, auto& output_sent);

    constexpr void EmitPosition(auto& output_iter, auto& output_sent);

    constexpr void EmitLength(auto& output_iter, auto& output_sent);

    static_assert(IsSymetric || IsAsymetric,
                  "All of the decoders has to be either symetric or asymetric");
};

}  // namespace koda

#include <koda/coders/lz77/lz77_intermediate_token_encoder.tpp>
