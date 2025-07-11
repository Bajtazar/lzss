#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/huffman/huffman_table.hpp>

#include <optional>

namespace koda {

template <typename Token,
          typename Allocator =
              std::allocator<typename HuffmanTable<Token>::entry_type>>
class HuffmanEncoder
    : public EncoderInterface<Token, HuffmanEncoder<Token, Allocator>> {
   public:
    constexpr explicit HuffmanEncoder(HuffmanTable<Token, Allocator> table);

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
        decltype(std::declval<std::vector<bool>>() | views::BigEndianInput);
    using BitIterator = std::ranges::iterator_t<BitRange>;
    using BitSentinel = std::ranges::iterator_t<BitSentinel>;

    HuffmanTable<Token, Allocator> table_;
    std::optional<std::pair<BitIterator, BitSentinel>> state_;
};

}  // namespace koda
