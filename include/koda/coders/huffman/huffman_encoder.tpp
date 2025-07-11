#pragma once

#include <koda/utils/utils.hpp>

#include <format>
#include <stdexcept>

namespace koda {

template <typename Token, typename Allocator>
constexpr HuffmanEncoder<Token, Allocator>::HuffmanEncoder(
    HuffmanTable<Token, Allocator> table)
    : table_{std::move(table)} {}

template <typename Token, typename Allocator>
constexpr float HuffmanEncoder<Token, Allocator>::TokenBitSize(
    Token token) const {
    if (auto iter = table_.Find(token); iter != table_.end()) [[likely]] {
        return iter->second.size();
    }
    throw std::runtime_error{std::format(
        "Token ({}) is not described by the huffman codes table", token)};
}

template <typename Token, typename Allocator>
constexpr auto HuffmanEncoder<Token, Allocator>::Encode(
    InputRange<Token> auto&& input, BitOutputRange auto&& output) {
    auto out = Flush(std::forward<decltype(output)>(output));
    if (!state_) {
        auto in_iter = std::ranges::begin(output);
        auto in_sent = std::ranges::end(output);
        auto out_iter = std::ranges::begin(output);
        auto out_sent = std::ranges::end(output);

        for (; (in_iter != in_sent) && (out_iter != out_sent); ++in_iter) {
            out_iter = EncodeToken(*in_iter, out_iter, out_sent);
        }

        return CoderResult{std::move(in_iter), std::move(in_sent),
                           std::move(out_iter), std::move(out_sent)};
    }
    return CoderResult{std::forward<decltype(input)>(input), std::move(out)};
}

template <typename Token, typename Allocator>
constexpr auto HuffmanEncoder<Token, Allocator>::Flush(
    BitOutputRange auto&& output) {
    if (state_) {
        auto& token_iter = state_->first;
        const auto& token_sent = state_->first;

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

template <typename Token, typename Allocator>
constexpr auto HuffmanEncoder<Token, Allocator>::EncodeToken(
    const Token& token, auto output_iter, const auto& output_sent) {
    auto symbol_iter = table_.Find(token);
    if (symbol_iter == table_.end()) [[unlikely]] {
        throw std::runtime_error{std::format(
            "Token ({}) is not described by the huffman codes table", token)};
    }
    auto range = symbol_iter->second | views::LittleEndianInput;
    state_ = BitPair{std::ranges::begin(range), std::ranges::end(range)};

    auto [res_iter, _] =
        Flush(std::ranges::subrange{std::move(output_iter), output_sent});
    return res_iter;
}

}  // namespace koda
