#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/huffman/huffman_table.hpp>

#include <memory>
#include <stdexcept>
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

        NodeOrLeaf left = nullptr;
        NodeOrLeaf right = nullptr;
    };

    using HuffmanTableEntry = typename HuffmanTable<Token>::entry_type;
    using NodeOrLeaf = Node::NodeOrLeaf;

    NodeOrLeaf root_;

    class TreeBuilder {
       public:
        constexpr TreeBuilder(const HuffmanTable<Token>& table)
            : root_{new Node{}} {
            // Populate unwinding table with original table
            ProcessUnwindingTableEntry(std::get<NodePtr>(root_).get(), table);
            ProcessUnwindingTable();
        }

        constexpr std::unique_ptr<Node> root() && { return std::move(root_); }

       private:
        std::unique_ptr<Node> root_;
        Map<Node*, std::vector<HuffmanTableEntry>> unwinding_table_;

        constexpr void ProcessUnwindingTableEntry(Node* entry_node,
                                                  const auto& entry_table) {
            std::vector<HuffmanTableEntry> left;
            std::vector<HuffmanTableEntry> right;

            for (auto [token, symbol] : entry_table) {
                if (symbol.empty()) [[unlikely]] {
                    throw std::runtime_error{"Invalid huffman table detected!"};
                }
                auto bit = symbol.front();
                symbol.erase(symbol.begin());
                if (bit) {
                    right.emplace_back(token, std::move(symbol));
                } else {
                    left.emplace_back(token, std::move(symbol));
                }
            }

            InsertUnwindingEntry(entry_node->left, std::move(left));
            InsertUnwindingEntry(entry_node->right, std::move(right));
        }

        constexpr void ProcessUnwindingTable() {
            while (!unwinding_table_.empty()) {
                auto entry_iter = unwinding_table_.begin();
                auto [parent, entry_table] = std::move(*entry_iter);
                unwinding_table_.Remove(entry_iter);

                ProcessUnwindingTableEntry(parent, entry_table);
            }
        }

        constexpr void InsertUnwindingEntry(
            NodeOrLeaf& hook, std::vector<HuffmanTableEntry>&& child_table) {
            if (child_table.empty()) {
                return;
            }
            if (child_table.size() == 1) {
                if (child_table.front().second.empty()) [[unlikely]] {
                    throw std::runtime_error{"Invalid huffman table detected!"};
                }
                hook = std::move(child_table.front().first);
                return;
            }
            std::unique_ptr<Node> new_child{new Node{}};
            unwinding_table_.Emplace(new_child.get(), std::move(child_table));
            hook = std::move(new_child);
        }
    };

    static constexpr NodeOrLeaf BuildTree(const HuffmanTable<Token>& table);
};

template <typename Token>
constexpr HuffmanDecoder<Token>::HuffmanDecoder(
    const HuffmanTable<Token>& table)
    : root_{BuildTree(table)} {}

template <typename Token>
/*static*/ constexpr HuffmanDecoder<Token>::NodeOrLeaf
HuffmanDecoder<Token>::BuildTree(const HuffmanTable<Token>& table) {
    if (table.size() == 1) {
        if (!table.begin()->second.empty()) [[unlikely]] {
            throw std::runtime_error{
                "For a distribution with only one element symbol should not "
                "exist"};
        }
        return table.begin()->first;
    }
    return TreeBuilder{table}.root();
}

}  // namespace koda
