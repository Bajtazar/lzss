#pragma once

#include <koda/collections/map.hpp>

#include <cinttypes>
#include <concepts>
#include <variant>
#include <vector>

namespace koda {

template <std::integral Token>
using HuffmanTable = Map<Token, std::vector<bool>>;

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
        if (NodePtr* root =
                std::get_if<NodePtr>(&work_table_.begin()->second.front())) {
            Node* node = FindLeftmost(root->get());
            AppendLeafNodes(node);

            while (node) {
                if (std::holds_alternative<NodePtr>(node->right)) {
                    symbols_.push_back(1);
                    node = FindLeftmost(std::get<NodePtr>(node->right).get());
                    AppendLeafNodes(node);
                    continue;
                }

                Node* previous;
                do {
                    previous = node;
                    node = node->parent;
                    symbols_.pop_back();
                } while (node && [&]() {
                    if (auto* ptr = std::get_if<NodePtr>(&node->right)) {
                        return ptr->get() == previous;
                    }
                    return false;
                }());
                if (node) {
                    AppendLeafNodes(node);
                }
            }
        } else {
            table_.Emplace(std::get<Token>(work_table_.begin()->second.front()),
                           std::vector<bool>{});
        }
        return std::move(table_);
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
            Emplace(occurences, NodeOrLeaf{token});
        }
    }

    constexpr void Emplace(CountTp occurences, NodeOrLeaf token) {
        if (auto iter = work_table_.Find(occurences);
            iter != work_table_.end()) {
            iter->second.emplace_back(std::move(token));
        } else {
            work_table_.Emplace(
                occurences,
                std::vector{std::move(token)});
        }
    }

    constexpr NodeOrLeaf ConcatenateNodes(NodeOrLeaf&& left,
                                          NodeOrLeaf&& right) {
        std::unique_ptr node{new Node{std::move(left), std::move(right)}};
        if (auto* left_node = std::get_if<NodePtr>(&left)) {
            left_node->parent = node.get();
        }
        if (auto* right_node = std::get_if<NodePtr>(&right)) {
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
        while (work_table_.size() > 1 || work_table_.front().size() > 1) {
            auto iter = work_table_.begin();
            if (iter->second.size() > 1) {
                EmplaceEquivariantSupernode(iter);
            } else {
                EmplaceSupernode(iter);
            }
        }
    }

    constexpr Node* FindLeftmost(Node* node) {
        for (; std::holds_alternative<NodePtr>(node->left);
             node = std::get<NodePtr>(node->left).get()) {
            symbols_.push_back(0);
        }
        return node;
    }

    constexpr void AppendLeafNodes(Node* node) {
        auto set_token_fn = [&](const NodeOrLeaf& leaf, bool bit) {
            if (const Token* token = std::get_if<Token>(&leaf)) {
                symbols_.push_back(bit);
                table_.Emplace(*token, symbols_);
                symbols_.pop_back();
            }
        };
        set_token_fn(node->left, 0);
        set_token_fn(node->right, 1);
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
