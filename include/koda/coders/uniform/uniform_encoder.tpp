#pragma once

#include <bit>

namespace koda {

template <std::integral Token>
constexpr UniformEncoder<Token>::UniformEncoder(size_t token_bit_size) noexcept
    : token_bit_size_{token_bit_size} {}

template <std::integral Token>
constexpr float UniformEncoder<Token>::TokenBitSize(Token token) const {
    return token_bit_size_;
}

template <std::integral Token>
constexpr auto UniformEncoder<Token>::Encode(InputRange<Token> auto&& input,
                                             BitOutputRange auto&& output) {
    auto sentinel = std::ranges::end(output);
    auto iter = FlushEmitter(std::ranges::begin(output), sentinel);

    if (iter == sentinel) {
        return CoderResult{std::forward<decltype(input)>(input),
                           std::move(iter), std::move(sentinel)};
    }

    return EncodeTokens(std::forward<decltype(input)>(input), std::move(iter),
                        std::move(sentinel));
}

template <std::integral Token>
constexpr auto UniformEncoder<Token>::Flush(BitOutputRange auto&& output) {
    auto sentinel = std::ranges::end(output);
    return std::ranges::subrange{
        FlushEmitter(std::ranges::begin(output), sentinel), sentinel};
}

template <std::integral Token>
constexpr auto UniformEncoder<Token>::EncodeTokens(
    InputRange<Token> auto&& input, auto iter, auto sentinel) {
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);

    for (; (iter != sentinel) && (input_iter != input_sent); ++input_iter) {
        SetEmitter(*input_iter);
        iter = FlushEmitter(iter, sentinel);
    }

    return CoderResult{std::move(input_iter), std::move(input_sent),
                       std::move(iter), std::move(sentinel)};
}

template <std::integral Token>
constexpr auto UniformEncoder<Token>::FlushEmitter(auto output_iter,
                                                   const auto& output_sent) {
    auto& bit_iter = emitter_.first;
    const auto& bit_sent = emitter_.second;

    for (; (bit_iter != bit_sent) && (output_iter != output_sent);
         ++output_iter, ++bit_iter) {
        *output_iter = *bit_iter;
    }
    return output_iter;
}

template <std::integral Token>
constexpr void UniformEncoder<Token>::SetEmitter(const Token& token) {
    token_[0] = token;
    emitter_ =
        std::pair{BitIter{std::ranges::begin(token_)},
                  token_bit_size_ == (sizeof(Token) * CHAR_BIT)
                      ? BitIter{std::ranges::end(token_)}
                      : BitIter{std::ranges::begin(token_), token_bit_size_}};
}

}  // namespace koda
