#pragma once

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr LzssIntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::LzssIntermediateTokenEncoder(TokenEncoder token_encoder,
                                                 PositionEncoder
                                                     position_encoder,
                                                 LengthEncoder length_encoder)
    : token_encoder_{std::move(token_encoder)},
      position_encoder_{std::move(position_encoder)},
      length_encoder_{std::move(length_encoder)} {}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr float LzssIntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::TokenBitSize(const token_type& token) const {
    if (auto symbol = token.get_symbol()) {
        return 1 + token_encoder_.TokenBitSize(*symbol);
    }
    auto [pos, len] = *token.get_marker();
    return 1 + position_encoder_.TokenBitSize(pos) +
           length_encoder_.TokenBitSize(len);
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr auto LzssIntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::Encode(InputRange<Token> auto&& input,
                           BitOutputRange auto&& output) {
    auto output_iter = std::ranges::begin(output);
    auto output_sent = std::ranges::end(output);
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);

    while ((input_iter != input_sent) && (output_iter != output_sent)) {
        switch (state_) {
            case State::kBit:
                EmitBit(*input_iter++, output_iter);
                break;
            case State::kToken:
                EmitToken(output_iter, output_sent);
                break;
            case State::kPosition:
                EmitPosition(output_iter, output_sent);
                break;
            case State::kLength:
                EmitLength(output_iter, output_sent);
                break;
        }
    }

    return CoderResult{std::move(input_iter), std::move(input_sent),
                       std::move(output_range)};
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr void LzssIntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::EmitBit(auto& token, auto& output_iter) {
    if (auto symbol = token.get_symbol()) {
        *output_iter++ = 0;
        state_ = State::kToken;
        emitter_.symbol[0] = *symbol;
    } else {
        *output_iter++ = 1;
        state = State::kPosition;
        auto [pos, len] = *token.get_marker();
        emitter_.marker.position[0] = pos;
        emitter_.marker.length[0] = len;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr void LzssIntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::EmitToken(auto& output_iter, auto& output_sent) {
    auto [in, out] = token_encoder_.Encode(
        std::ranges::subrange{std::ranges::begin(emitter_.symbol),
                              std::ranges::end(emitter_.symbol)},
        std::ranges::subrange{output_iter, output_sent});

    output_iter = std::ranges::begin(out);
    output_sent = std::ranges::end(out);

    if (in.empty()) {
        state_ = State::kBit;
        emitter_.symbol[0].~InputToken();
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr void LzssIntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::EmitPosition(auto& output_iter, auto& output_sent) {
    auto [in, out] = position_encoder_.Encode(
        std::ranges::subrange{std::ranges::begin(emitter_.position),
                              std::ranges::end(emitter_.position)},
        std::ranges::subrange{output_iter, output_sent});

    output_iter = std::ranges::begin(out);
    output_sent = std::ranges::end(out);

    if (in.empty()) {
        state_ = State::kLength;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr void LzssIntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::EmitLength(auto& output_iter, auto& output_sent) {
    auto [in, out] = position_encoder_.Encode(
        std::ranges::subrange{std::ranges::begin(emitter_.length),
                              std::ranges::end(emitter_.length)},
        std::ranges::subrange{output_iter, output_sent});

    output_iter = std::ranges::begin(out);
    output_sent = std::ranges::end(out);

    if (in.empty()) {
        state_ = State::kBit;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr auto LzssIntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::Flush(BitOutputRange auto&& output) {
    auto output_iter = std::ranges::begin(output);
    auto output_sent = std::ranges::end(output);

    auto flush = [&](auto& encoder) {
        auto res =
            encoder.Flush(std::ranges::subrange{output_iter, output_sent});
        output_iter = std::ranges::begin(res);
        output_sent = std::ranges::end(res);
    };

    flush(token_encoder_);
    flush(position_encoder_);
    flush(length_encoder_);

    return std::ranges::subrange{std::move(output_iter),
                                 std::move(output_sent)};
}

}  // namespace koda
