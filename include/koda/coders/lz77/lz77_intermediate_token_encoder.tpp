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
    LengthEncoder>::TokenBitSize(const token_type& token) const {
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
    LengthEncoder>::Encode(InputRange<token_type> auto&& input,
                           BitOutputRange auto&& output) {
    auto output_iter = std::ranges::begin(output);
    auto output_sent = std::ranges::end(output);
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);

    while (((input_iter != input_sent) || state_ != State::kInsert) &&
           (output_iter != output_sent)) {
        switch (state_) {
            case State::kInsert:
                InsertSymbol(*input_iter++);
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
                       std::move(output_iter), std::move(output_sent)};
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr auto Lz77IntermediateTokenEncoder<
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

    if constexpr (IsSymetric) {
        flush(token_encoder_);
        flush(position_encoder_);
        flush(length_encoder_);
    } else {
        flush(length_encoder_);
        flush(position_encoder_);
        flush(token_encoder_);
    }

    return std::ranges::subrange{std::move(output_iter),
                                 std::move(output_sent)};
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr void Lz77IntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::InsertSymbol(const token_type& token) {
    symbol_[0] = token.suffix_symbol();
    position_[0] = token.match_position();
    length_[0] = token.match_length();
    state_ = IsSymetric ? State::kToken : State::kLength;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr void Lz77IntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::EmitToken(auto& output_iter, auto& output_sent) {
    auto [in, out] =
        token_encoder_.Encode(std::ranges::subrange{std::ranges::begin(symbol_),
                                                    std::ranges::end(symbol_)},
                              std::ranges::subrange{output_iter, output_sent});

    output_iter = std::ranges::begin(out);
    output_sent = std::ranges::end(out);

    if (in.empty()) {
        state_ = IsSymetric ? State::kPosition : State::kInsert;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr void Lz77IntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::EmitPosition(auto& output_iter, auto& output_sent) {
    auto [in, out] = position_encoder_.Encode(
        std::ranges::subrange{std::ranges::begin(position_),
                              std::ranges::end(position_)},
        std::ranges::subrange{output_iter, output_sent});

    output_iter = std::ranges::begin(out);
    output_sent = std::ranges::end(out);

    if (in.empty()) {
        state_ = IsSymetric ? State::kLength : State::kToken;
    }
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp, SizeAwareEncoder<InputToken> TokenEncoder,
          SizeAwareEncoder<PositionTp> PositionEncoder,
          SizeAwareEncoder<LengthTp> LengthEncoder>
constexpr void Lz77IntermediateTokenEncoder<
    InputToken, PositionTp, LengthTp, TokenEncoder, PositionEncoder,
    LengthEncoder>::EmitLength(auto& output_iter, auto& output_sent) {
    auto [in, out] = length_encoder_.Encode(
        std::ranges::subrange{std::ranges::begin(length_),
                              std::ranges::end(length_)},
        std::ranges::subrange{output_iter, output_sent});

    output_iter = std::ranges::begin(out);
    output_sent = std::ranges::end(out);

    if (in.empty()) {
        state_ = IsSymetric ? State::kInsert : State::kPosition;
    }
}

}  // namespace koda
