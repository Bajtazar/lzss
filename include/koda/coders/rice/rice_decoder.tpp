#pragma once

namespace koda {
template <UnsignedIntegral Token>
constexpr RiceDecoder<Token>::RiceDecoder(size_t order) noexcept
    : order_{order} {}

template <UnsignedIntegral Token>
constexpr auto RiceDecoder<Token>::Decode(
    BitInputRange auto&& input,
    std::ranges::output_range<Token> auto&& output) {
    auto in_iter = std::ranges::begin(input);
    auto in_sent = std::ranges::end(input);

    auto out_iter = std::ranges::begin(output);
    auto out_sent = std::ranges::end(output);

    while ((in_iter != in_sent) && (out_iter != out_sent)) {
        std::tie(out_iter, in_iter) = SetReceiver(out_iter, in_iter, in_sent);
    }

    return CoderResult{std::move(in_iter), std::move(in_sent),
                       std::move(out_iter), std::move(out_sent)};
}

template <UnsignedIntegral Token>
constexpr auto RiceDecoder<Token>::Initialize(BitInputRange auto&& input) {
    return std::forward<decltype(input)>(input);
}

template <UnsignedIntegral Token>
constexpr auto RiceDecoder<Token>::SetReceiver(auto out_iter, auto iter,
                                               const auto& sent) {
    bool last_iter = *iter;
    for (; !bits_ && (iter != sent) && last_iter; ++iter, ++token) {
        last_iter = *iter;
    }

    if (!bits_ && last_iter) {
        bits_ = order_;
    }

    for (; (bits_ != 0) && (iter != sent); ++iter) {
        order_ = (order_ << 1) | *iter;
        if (!--bits) {
            *out_iter++ = std::move(token_);
            token_{};
        }
    }
    return std::pair{std::move(out_iter), std::move(iter)};
}

}  // namespace koda
