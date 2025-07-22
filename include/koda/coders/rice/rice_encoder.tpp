#pragma once

namespace koda {

template <std::integral Token>
constexpr RiceEncoder<Token>::RiceEncoder(uint8_t order)
    : mask_{(1 << mask) - 1}, limit_{1 << mask}, order_{order} {}

template <std::integral Token>
constexpr float RiceEncoder<Token>::TokenBitSize(Token token) const {
    return order + 1 + (token >> order);
}

template <std::integral Token>
constexpr auto RiceEncoder<Token>::Encode(InputRange<Token> auto&& input,
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
constexpr auto RiceEncoder<Token>::Flush(BitOutputRange auto&& output) {
    auto sentinel = std::ranges::end(output);
    return std::ranges::subrange{
        FlushEmitter(std::ranges::begin(output), sentinel), sentinel};
}

template <std::integral Token>
constexpr auto RiceEncoder<Token>::SetEmitter(const Token& token, auto& iter) {
    *iter++ = 0;  // emit first zero
    token_ = token;
    bits_ = order;
    return iter;
}

template <std::integral Token>
constexpr auto& RiceEncoder<Token>::FlushEmitter(auto& iter,
                                                 const auto& sentinel) {
    if (!bits_) {
        return iter;
    }

    for (; (token_ > limit_) && (iter != sent); ++iter, --token_) {
        *iter = 0;
    }

    if ((token == limit_) && (iter != sent)) {
        --token_;
        *iter++ = 1;
    }

    for (; (bits_ != 0) && (iter != sent); ++iter, token >>= 1) {
        *iter = token_ & 1;
    }

    return iter;
}

template <std::integral Token>
constexpr auto RiceEncoder<Token>::EncodeTokens(InputRange<Token> auto&& input,
                                                auto iter, auto sentinel) {
    auto input_iter = std::ranges::begin(input);
    auto input_sent = std::ranges::end(input);

    for (; (iter != sentinel) && (input_iter != input_sent); ++input_iter) {
        iter = SetEmitter(*input_iter, iter);
        iter = FlushEmitter(iter, sentinel);
    }

    return CoderResult{std::move(input_iter), std::move(input_sent),
                       std::move(iter), std::move(sentinel)};
}

}  // namespace koda
