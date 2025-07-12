#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/huffman/huffman_table.hpp>

#include <memory>
#include <variant>
#include <vector>

namespace koda {

template <typename Token>
class HuffmanDecoder : public DecoderInterface<Token, HuffmanDecoder<Token>> {
   public:
    constexpr explicit HuffmanDecoder(const HuffmanTable<Token>& table);

    constexpr HuffmanDecoder(HuffmanDecoder&& other) noexcept = default;
    constexpr HuffmanDecoder(const HuffmanDecoder& other) noexcept = delete;

    constexpr HuffmanDecoder& operator=(HuffmanDecoder&& other) noexcept =
        default;
    constexpr HuffmanDecoder& operator=(const HuffmanDecoder& other) noexcept =
        delete;

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

    constexpr auto Initialize(BitInputRange auto&& input);

    constexpr ~HuffmanDecoder() = default;

   private:
    struct Node {
        using NodeOrLeaf = std::variant<std::unique_ptr<Node>, Token>;

        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    };

    using UnwindEntry = std::pair<Node*, std::vector<Token>>;
    using NodeOrLeaf = Node::NodeOrLeaf;

    NodeOrLeaf root_;

    class TreeBuilder;

    static constexpr NodeOrLeaf BuildTree(const HuffmanTable<Token>& table);
};

}  // namespace koda
