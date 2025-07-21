#pragma once

#include <koda/utils/utils.hpp>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr Lz77IntermediateTokenDeoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::Lz77IntermediateTokenDeoder(TokenDecoder token_decoder,
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
    auto input_range = AsSubrange(std::forward<decltype(input)>(input));

    while (!input_range.empty()) {
        // std::tie doesn't work on structs
        input_range = [&] {
            switch (state_) {
                case State::kToken:
                    return token_decoder_.Initialize(input);
                case State::kPosition:
                    return position_decoder_.Initialize(input);
                case State::kToken:
                    return length_decoder_.Initialize(input);
            }
        }();
        state_ = kNextState[state_];
    }

    return output_range;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, Decoder<InputToken> TokenDecoder,
          Decoder<PositionTp> PositionDecoder, Decoder<LengthTp> LengthDecoder>
constexpr auto Lz77IntermediateTokenDeoder<
    InputToken, PositionTp, LengthTp, TokenDecoder, PositionDecoder,
    LengthDecoder>::Decode(BitInputRange auto&& input,
                           std::ranges::output_range<Token> auto&& output) {
    auto input_range = AsSubrange(std::forward<decltype(input)>(input));
    auto output_range = AsSubrange(std::forward<decltype(output)>(output));

    while (!input_range.empty() && !output_range.empty()) {
        // std::tie doesn't work on structs
        auto [new_input, new_output] = [&] {
            switch (state_) {
                case State::kToken:
                    return token_decoder_.Decode(input_range, output_range);
                case State::kPosition:
                    return position_decoder_.Decode(input_range, output_range);
                case State::kLength:
                    return length_decoder_.Decode(input_range, output_range);
            }
        }();
        state_ = kNextState[state_];
        input_range = std::move(new_input);
        output_range = std::move(new_output);
    }

    return CoderResult{std::move(input_range), std::move(output_range)};
}

}  // namespace koda
