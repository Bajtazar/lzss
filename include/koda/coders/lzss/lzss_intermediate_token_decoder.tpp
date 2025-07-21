#pragma once

#include <utility>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr LzssIntermediateTokenDeoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::LzssIntermediateTokenDeoder(TokenDecoder token_decoder,
                                                PositionDecoder
                                                    position_decoder,
                                                LengthDecoder length_decoder)
    : token_decoder_{std::move(token_decoder)},
      position_decoder_{std::move(position_decoder)},
      length_decoder_{std::move(length_decoder)} {}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr auto Lz77IntermediateTokenDeoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::Initialize(BitInputRange auto&& input) {
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);

    auto init = [&](auto& decoder) {
        auto res =
            decoder.Initialize(std::ranges::subrange{input_iter, input_sent});
        input_iter = std::ranges::begin(res);
        input_sent = std::ranges::end(res);
    };

    init(token_decoder_);
    init(position_decoder_);
    init(length_decoder_);

    return output_range;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr auto Lz77IntermediateTokenDeoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::Decode(BitInputRange auto&& input,
                           std::ranges::output_range<Token> auto&& output) {
    auto output_iter = std::ranges::begin(output);
    auto output_sent = std::ranges::end(output);
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);

    while ((input_iter != input_sent) && (output_iter != output_sent)) {
        switch (state_) {
            case State::kBit:
                ReceiveBit(*input_iter++);
                break;
            case State::kToken:
                ReceiveToken(input_iter, input_sent, output_iter);
                break;
            case State::kPosition:
                ReceivePosition(input_iter, input_sent);
                break;
            case State::kLength:
                ReceiveLength(input_iter, input_sent, output_iter);
                break;
        }
    }

    return CoderResult{std::move(input_iter), std::move(input_sent),
                       std::move(output_range)};
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr void LzssIntermediateTokenDeoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::ReceiveBit(bool bit) {
    state_ = bit ? State::kPosition : State::kToken;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr void LzssIntermediateTokenDeoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::ReceiveToken(auto& input_iter, auto& input_sent,
                                 auto& output_iter) {
    Token token[0];

    auto [in, out] =
        token_decoder_.Decode(std::ranges::subrange{input_iter, input_sent},
                              std::ranges::subrange{std::ranges::begin(token),
                                                    std::ranges::end(token)});

    input_iter = std::ranges::begin(in);
    input_sent = std::ranges::end(in);

    if (!out.empty()) {
        ++output_iter++ = token_type{std::move(token[0])};
        state_ = State::kBit;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr void LzssIntermediateTokenDeoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::ReceivePosition(auto& input_iter, auto& input_sent) {
    auto [in, out] = token_decoder_.Decode(
        std::ranges::subrange{input_iter, input_sent},
        std::ranges::subrange{std::ranges::begin(receiver_position_),
                              std::ranges::end(receiver_position_)});

    input_iter = std::ranges::begin(in);
    input_sent = std::ranges::end(in);

    if (!out.empty()) {
        state_ = State::kLength;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr void LzssIntermediateTokenDeoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::ReceiveLength(auto& input_iter, auto& input_sent,
                                  auto& output_iter) {
    LengthTp length[0];

    auto [in, out] =
        token_decoder_.Decode(std::ranges::subrange{input_iter, input_sent},
                              std::ranges::subrange{std::ranges::begin(length),
                                                    std::ranges::end(length)});

    input_iter = std::ranges::begin(in);
    input_sent = std::ranges::end(in);

    if (!out.empty()) {
        ++output_iter++ = token_type{receiver_position_[0], length[0]};
        state_ = State::kBit;
    }
}

}  // namespace koda
