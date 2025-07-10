#pragma once

#include <koda/collections/map.hpp>

#include <cinttypes>
#include <concepts>
#include <variant>
#include <vector>

namespace koda {

struct HuffmanTableEntryComparator {
    template <std::integral Token>
    [[nodiscard]] static constexpr WeakOrdering operator()(
        const std::pair<const Token, std::vector<bool>>& left,
        const std::pair<const Token, std::vector<bool>>& right) noexcept;
};

template <std::integral Token,
          typename AllocatorTp =
              std::allocator<std::pair<const Token, std::vector<bool>>>>
using HuffmanTable =
    Map<Token, std::vector<bool>, HuffmanTableEntryComparator, AllocatorTp>;

template <std::integral Token, std::integral CountTp>
[[nodiscard]] constexpr HuffmanTable<Token> MakeHuffmanTable(
    const Map<Token, CountTp>& count);

namespace details {

template <std::integral Token, std::integral CountTp>
class MakeHuffmanTableFn {
   public:
    explicit constexpr MakeHuffmanTableFn(const Map<Token, CountTp>& count) {
        InitializeWorkTable(count);
    }

    constexpr HuffmanTable<Token>&& table() && {
        if (Node* node = std::get_if<Node>(work_table_.begin()->second)) {
            node = FindLeftmost(node);
            AppendLeafNodes(node);

            while (node) {
                if (std::holds_alternative<Node>(node->right)) {
                    symbol.push_back(1);
                    node = FindLeftmost(std::get<Node>(node->right));
                    AppendLeafNodes(node);
                    continue;
                }

                Node* previous;
                do {
                    previous = node;
                    node = node->parent;
                    symbol.pop_back();
                } while (node && previous == std::get_if<Node>(node->right));
                if (node) {
                    AppendLeafNodes(node);
                }
            }
            return std::move(table_);
        }
        return HuffmanTable<Token>{std::pair{
            std::get<Token>(work_table_.begin()->second), std::vector<bool>{}}};
    }

   private:
    struct Node {
        using NodeOrLeaf = std::variant<std::unique_ptr<Node>, Token>;

        NodeOrLeaf left;
        NodeOrLeaf right;
        Node* parent = nullptr;
    };

    using NodeOrLeaf = Node::NodeOrLeaf;
    using NodePtr = std::unique_ptr<Node>;

    Map<CountTp, std::vector<NodeOrLeaf>> work_table_;
    std::vector<bool> symbols_;
    HuffmanTable<Token> table_;

    constexpr void InitializeWorkTable(const Map<Token, CountTp>& count) {
        for (const auto& [token, occurences] : count) {
            Emplace(occurences, {token});
        }
    }

    constexpr void Emplace(CountTp occurences, NodeOrLeaf token) {
        if (auto iter = work_table_.Find(occurences);
            iter != work_table_.end()) {
            iter->second.emplace_back(std::move(token));
        } else {
            work_table_.Emplace(occurences, std::move(token));
        }
    }

    constexpr NodeOrLeaf ConcatenateNodes(NodeOrLeaf&& left,
                                          NodeOrLeaf&& right) {
        std::unique_ptr node{new Node{std::move(left), std::move(right)}};
        if (auto* left_node = std::get_if<Node>(&left)) {
            left_node->parent = node.get();
        }
        if (auto* right_node = std::get_if<Node>(&right)) {
            right_node->parent = node.get();
        }
        return node;
    }

    constexpr void RemoveElementsFromEquivariantNodes(const auto& iter,
                                                      size_t length = 1) {
        auto& equivariant = iter->second;
        equivariant.erase(equivariant.begin(),
                          std::next(equivariant.begin(), 2));
        if (equivariant.empty()) {
            work_table_.Remove(iter);
        }
    }

    constexpr void EmplaceEquivariantSupernode(const auto& iter) {
        auto& least_common = iter->second;
        auto new_node = ConcatenateNodes(least_common[0], least_common[1]);
        auto new_occur = 2 * iter->first;
        RemoveElementsFromEquivariantNodes(iter, 2);
        Emplace(new_occur, std::move(new_node));
    }

    constexpr void EmplaceSupernode(const auto& iter) {
        auto& least_common = iter->second;
        auto least_common_occur = iter->first;
        auto& second_least_common = (++iter)->second;

        auto new_node =
            ConcatenateNodes(least_common[0], second_least_common[0]);
        auto new_occur = least_common_occur + iter->first;
        RemoveElementsFromEquivariantNodes(iter, 1);
        work_table_.Remove(least_common_occur);
        Emplace(new_occur, std::move(new_node));
    }

    constexpr void ProcessWorkTable() {
        // Map sort elements in ascending order
        while (work_table_.size() > 1) {
            auto iter = work_table_.begin();
            if (iter->second.size() > 1) {
                EmplaceEquivariantSupernode(iter);
            } else {
                EmplaceSupernode(iter);
            }
        }
    }

    constexpr Node* FindLeftmost(Node* node) {
        for (; std::holds_alternative<Node>(node->left);
             node = std::get<Node>(node->left)) {
            symbols_.push_back(0);
        }
        return node;
    }

    constexpr void AppendLeafNodes(Node* node) {
        auto set_token_fn = [&](const NodeOrLeaf& leaf, bool bit) {
            if (Token* token = std::get_if<Token>(&leaf)) {
                symbols_.push_back(bit);
                table_.Emplace(*token, symbols_);
                symbols_.pop_back();
            }
        };
        set_token_fn(node->left, 0);
        set_token_fn(node->left, 1);
    }
};

}  // namespace details

template <std::integral Token, std::integral CountTp>
[[nodiscard]] constexpr HuffmanTable<Token> MakeHuffmanTable(
    const Map<Token, CountTp>& count) {
    return details::MakeHuffmanTableFn{count}.table();
}

}  // namespace koda

#include <koda/coders/huffman/huffman_table.tpp>
