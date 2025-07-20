#pragma once

#include <bit>

namespace koda {

template <std::integral Token>
[[nodiscard]] /*static*/ consteval size_t
UniformEncoderTraits<Token>::TokenMaxByteSize() noexcept {
    return sizeof(Token);
}

template <std::integral Token>
template <size_t Size>
/*static*/ constexpr uint8_t UniformEncoderTraits<Token>::PopulateBuffer(
    std::array<uint8_t, Size>& array, const Token& token) {
    size_t length = 0;
    for (auto token_cpy = token; length != TokenMaxByteSize();
         ++length, token_cpy >>= CHAR_BIT) {
        array[length] = token_cpy & 0xff;
    }
    return length * CHAR_BIT;
}

template <typename Token>
constexpr float UniformEncoder<Token>::TokenBitSize(Token token) const {
    return Traits::TokenMaxByteSize() * CHAR_BIT;
}

template <typename Token>
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

template <typename Token>
constexpr auto UniformEncoder<Token>::Flush(BitOutputRange auto&& output) {
    auto sentinel = std::ranges::end(output);
    return std::ranges::subrange{
        FlushEmitter(std::ranges::begin(output), sentinel), sentinel};
}

template <typename Token>
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

template <typename Token>
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

template <typename Token>
constexpr void UniformEncoder<Token>::SetEmitter(const Token& token) {
    auto bit_count = Traits::PopulateBuffer(buffer_, token);
    auto shift = bit_count / CHAR_BIT;
    emitter_ =
        std::pair{BitIter{&buffer_[0]},
                  BitIter{shift != buffer_.size() ? &buffer_[shift]
                                                  : std::ranges::end(buffer_),
                          static_cast<size_t>(bit_count % CHAR_BIT)}};
}

}  // namespace koda
