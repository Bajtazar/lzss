#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/huffman/huffman_table.hpp>

#include <optional>

namespace koda {

template <typename Token>
class HuffmanEncoder : public EncoderInterface<Token, HuffmanEncoder<Token>> {
   public:
    constexpr explicit HuffmanEncoder(HuffmanTable<Token> table);

    constexpr HuffmanEncoder(HuffmanEncoder&& other) noexcept = default;
    constexpr HuffmanEncoder(const HuffmanEncoder& other) noexcept = delete;

    constexpr HuffmanEncoder& operator=(HuffmanEncoder&& other) noexcept =
        default;
    constexpr HuffmanEncoder& operator=(const HuffmanEncoder& other) noexcept =
        delete;

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

    constexpr ~HuffmanEncoder() = default;

   private:
    using BitRange =
        decltype(std::declval<std::vector<bool>>() | views::LittleEndianInput);
    using BitIterator = std::ranges::iterator_t<BitRange>;
    using BitSentinel = std::ranges::iterator_t<BitSentinel>;
    using BitPair = std::pair<BitIterator, BitSentinel>;

    HuffmanTable<Token> table_;
    std::optional<std::pair<BitIterator, BitSentinel>> state_;

    constexpr auto EncodeToken(const Token& token, auto output_iter,
                               const auto& output_sent);
};

}  // namespace koda

#include <koda/coders/huffman/huffman_encoder.tpp>
