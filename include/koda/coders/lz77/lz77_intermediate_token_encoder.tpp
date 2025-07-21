#pragma once

#include <koda/utils/utils.hpp>

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr Lz77IntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::Lz77IntermediateTokenEncoder(TokenEncoder token_encoder,
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
constexpr float Lz77IntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::TokenBitSize(Token token) const {
    return token_encoder_.TokenBitSize(token.suffix_symbol()) +
           position_encoder_.TokenBitSize(token.match_position()) +
           length_encoder_.TokenBitSize(token.match_length());
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr auto Lz77IntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::Encode(InputRange<Token> auto&& input,
                           BitOutputRange auto&& output) {
    auto input_range = AsSubrange(std::forward<decltype(input)>(input));
    auto output_range = AsSubrange(std::forward<decltype(output)>(output));

    while (!input_range.empty() && !output_range.empty()) {
        // std::tie doesn't work on structs
        auto [new_input, new_output] = [&] {
            switch (state_) {
                case State::kToken:
                    return token_encoder_.Encode(input_range, output_range);
                case State::kPosition:
                    return position_encoder_.Encode(input_range, output_range);
                case State::kLength:
                    return length_encoder_.Encode(input_range, output_range);
            }
        }();
        state_ = kNextState[state_];
        input_range = std::move(new_input);
        output_range = std::move(new_output);
    }

    return CoderResult{std::move(input_range), std::move(output_range)};
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr auto Lz77IntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::Flush(BitOutputRange auto&& output) {
    auto output_range = AsSubrange(std::forward<decltype(output)>(output));

    while (!output_range.empty()) {
        // std::tie doesn't work on structs
        output_range = [&] {
            switch (state_) {
                case State::kToken:
                    return token_encoder_.Flush(output_range);
                case State::kPosition:
                    return position_encoder_.Flush(output_range);
                case State::kToken:
                    return length_encoder_.Flush(output_range);
            }
        }();
        state_ = kNextState[state_];
    }

    return output_range;
}

}  // namespace koda
