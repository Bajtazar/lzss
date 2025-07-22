#pragma once

#include <utility>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr LzssIntermediateTokenDecoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::LzssIntermediateTokenDecoder(TokenDecoder token_decoder,
                                                 PositionDecoder
                                                     position_decoder,
                                                 LengthDecoder length_decoder)
    : token_decoder_{std::move(token_decoder)},
      position_decoder_{std::move(position_decoder)},
      length_decoder_{std::move(length_decoder)} {}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr auto LzssIntermediateTokenDecoder<
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

    return std::ranges::subrange{std::move(input_iter), std::move(input_sent)};
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr auto LzssIntermediateTokenDecoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::Decode(BitInputRange auto&& input,
                           std::ranges::output_range<token_type> auto&&
                               output) {
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
                       std::move(output_iter), std::move(output_sent)};
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr void LzssIntermediateTokenDecoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::ReceiveBit(bool bit) {
    state_ = bit ? State::kPosition : State::kToken;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr void LzssIntermediateTokenDecoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::ReceiveToken(auto& input_iter, auto& input_sent,
                                 auto& output_iter) {
    InputToken token[1];

    auto [in, out] =
        token_decoder_.Decode(std::ranges::subrange{input_iter, input_sent},
                              std::ranges::subrange{std::ranges::begin(token),
                                                    std::ranges::end(token)});

    input_iter = std::ranges::begin(in);
    input_sent = std::ranges::end(in);

    if (out.empty()) {
        *output_iter++ = token_type{std::move(token[0])};
        state_ = State::kBit;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr void LzssIntermediateTokenDecoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::ReceivePosition(auto& input_iter, auto& input_sent) {
    auto [in, out] = position_decoder_.Decode(
        std::ranges::subrange{input_iter, input_sent},
        std::ranges::subrange{std::ranges::begin(receiver_position_),
                              std::ranges::end(receiver_position_)});

    input_iter = std::ranges::begin(in);
    input_sent = std::ranges::end(in);

    if (out.empty()) {
        state_ = State::kLength;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr void LzssIntermediateTokenDecoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::ReceiveLength(auto& input_iter, auto& input_sent,
                                  auto& output_iter) {
    LengthTp length[1];

    auto [in, out] =
        length_decoder_.Decode(std::ranges::subrange{input_iter, input_sent},
                               std::ranges::subrange{std::ranges::begin(length),
                                                     std::ranges::end(length)});

    input_iter = std::ranges::begin(in);
    input_sent = std::ranges::end(in);

    if (out.empty()) {
        // Encoder decreases length by one since 0 is not permitted
        *output_iter++ = token_type{receiver_position_[0], ++length[0]};
        state_ = State::kBit;
    }
}

}  // namespace koda
