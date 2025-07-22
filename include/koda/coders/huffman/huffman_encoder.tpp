#pragma once

#include <koda/utils/formatted_exception.hpp>
#include <koda/utils/utils.hpp>

namespace koda {

template <typename Token>
constexpr HuffmanEncoder<Token>::HuffmanEncoder(HuffmanTable<Token> table)
    : table_{std::move(table)} {}

template <typename Token>
constexpr float HuffmanEncoder<Token>::TokenBitSize(Token token) const {
    if (auto iter = table_.Find(token); iter != table_.end()) [[likely]] {
        return iter->second.size();
    }
    ThrowException(token);
}

template <typename Token>
constexpr auto HuffmanEncoder<Token>::Encode(InputRange<Token> auto&& input,
                                             BitOutputRange auto&& output) {
    auto out = Flush(std::forward<decltype(output)>(output));
    if (!state_) {
        auto in_iter = std::ranges::begin(input);
        auto in_sent = std::ranges::end(input);
        auto out_iter = std::ranges::begin(out);
        auto out_sent = std::ranges::end(out);

        for (; (in_iter != in_sent) && (out_iter != out_sent); ++in_iter) {
            out_iter = EncodeToken(*in_iter, out_iter, out_sent);
        }

        return CoderResult{std::move(in_iter), std::move(in_sent),
                           std::move(out_iter), std::move(out_sent)};
    }
    return CoderResult{std::forward<decltype(input)>(input), std::move(out)};
}

template <typename Token>
constexpr auto HuffmanEncoder<Token>::Flush(BitOutputRange auto&& output) {
    if (state_) {
        auto& token_iter = state_->first;
        const auto& token_sent = state_->second;

        auto out_iter = std::ranges::begin(output);
        auto out_sent = std::ranges::end(output);
        for (; (token_iter != token_sent) && (out_iter != out_sent);
             ++token_iter, ++out_iter) {
            *out_iter = *token_iter;
        }

        if (token_iter == token_sent) {
            state_ = std::nullopt;
        }
        return std::ranges::subrange{std::move(out_iter), std::move(out_sent)};
    }
    return AsSubrange(std::forward<decltype(output)>(output));
}

template <typename Token>
constexpr auto HuffmanEncoder<Token>::EncodeToken(const Token& token,
                                                  auto output_iter,
                                                  const auto& output_sent) {
    auto symbol_iter = table_.Find(token);
    if (symbol_iter == table_.end()) [[unlikely]] {
        ThrowException(token);
    }
    state_ = std::pair{symbol_iter->second.begin(), symbol_iter->second.end()};

    auto [res_iter, _] =
        Flush(std::ranges::subrange{std::move(output_iter), output_sent});
    return res_iter;
}

template <typename Token>
[[noreturn]] constexpr void HuffmanEncoder<Token>::ThrowException(
    Token token) const {
    throw FormattedException{
        "Token ({}) is not described by the huffman codes table", token};
}

}  // namespace koda
