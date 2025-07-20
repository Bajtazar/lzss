#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/huffman/huffman_table.hpp>
#include <koda/collections/forward_list.hpp>

#include <memory>
#include <stdexcept>
#include <variant>
#include <vector>

namespace koda {

template <typename Token>
class HuffmanDecoder : public DecoderInterface<Token, HuffmanDecoder<Token>> {
   public:
    using token_type = Token;

    constexpr explicit HuffmanDecoder(const HuffmanTable<Token>& table);

    constexpr HuffmanDecoder(HuffmanDecoder&& other) noexcept = default;
    constexpr HuffmanDecoder(const HuffmanDecoder& other) noexcept = delete;

    constexpr HuffmanDecoder& operator=(HuffmanDecoder&& other) noexcept =
        default;
    constexpr HuffmanDecoder& operator=(const HuffmanDecoder& other) noexcept =
        delete;

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

    constexpr auto Initialize(BitInputRange auto&& input);

    constexpr ~HuffmanDecoder() = default;

   private:
    struct Node {
        using NodeOrLeaf = std::variant<std::unique_ptr<Node>, Token>;

        NodeOrLeaf left = nullptr;
        NodeOrLeaf right = nullptr;
    };

    using NodePtr = std::unique_ptr<Node>;
    using HuffmanTableEntry = typename HuffmanTable<Token>::entry_type;
    using NodeOrLeaf = Node::NodeOrLeaf;

    NodeOrLeaf root_;
    const Node* processed_;

    class TreeBuilder {
       public:
        constexpr TreeBuilder(const HuffmanTable<Token>& table);

        constexpr NodePtr root() &&;

       private:
        NodePtr root_;
        ForwardList<std::pair<Node*, std::vector<HuffmanTableEntry>>>
            unwinding_table_;
        size_t counter_ = 0;

        constexpr void ProcessUnwindingTableEntry(Node* entry_node,
                                                  const auto& entry_table);

        constexpr void ProcessUnwindingTable();

        constexpr void InsertUnwindingEntry(
            NodeOrLeaf& hook, std::vector<HuffmanTableEntry>&& child_table);
    };

    constexpr auto HandleDiracDistribution(
        const Token& token, BitInputRange auto&& input,
        std::ranges::output_range<Token> auto&& output);

    constexpr auto DecodeNonDirac(
        BitInputRange auto&& input,
        std::ranges::output_range<Token> auto&& output);

    constexpr void ProcessBit(auto& output_iter, const NodeOrLeaf& next);

    static constexpr NodeOrLeaf BuildTree(const HuffmanTable<Token>& table);
};

}  // namespace koda

#include <koda/coders/huffman/huffman_decoder.tpp>
