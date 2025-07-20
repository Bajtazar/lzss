#pragma once

namespace koda {

template <std::integral Token>
constexpr UniformDecoder<Token>::UniformDecoder(uint8_t token_bit_size) noexcept
    : token_bit_size_{token_bit_size} {}

template <std::integral Token>
constexpr auto UniformDecoder<Token>::Decode(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    auto in_iter = std::ranges::begin(input);
    auto in_sent = std::ranges::end(input);

    auto out_iter = std::ranges::begin(output);
    auto out_sent = std::ranges::end(output);

    for (; (in_iter != in_sent) && (out_iter != out_sent); ++out_iter) {
        in_iter = SetReceiver(in_iter, in_sent);
        if (receiver_.first == receiver_.second) {
            *out_iter = DecodeToken();
        }
    }

    return CoderResult{std::move(in_iter), std::move(in_iter),
                       std::move(out_iter), std::move(out_sent)};
}

template <std::integral Token>
constexpr auto UniformDecoder<Token>::Initialize(BitInputRange auto&& input) {
    return std::forward<decltype(input)>(input);
}

template <std::integral Token>
constexpr auto UniformDecoder<Token>::SetReceiver(auto iter, const auto& sent) {
    auto& state_iter = receiver_.first;
    const auto& state_sent = receiver_.second;

    for (; (iter != sent) && (state_iter != state_sent); ++iter, ++state_iter) {
        *state_iter = *iter;
    }

    return iter;
}

template <std::integral Token>
constexpr Token UniformDecoder<Token>::DecodeToken() {
    Token token = token_[0];
    receiver_ =
        std::pair{BitIter{std::ranges::begin(token_)},
                  token_bit_size_ == (sizeof(Token) * CHAR_BIT)
                      ? BitIter{std::ranges::end(token_)}
                      : BitIter{std::ranges::begin(token_), token_bit_size_}};
    return token;
}

}  // namespace koda
