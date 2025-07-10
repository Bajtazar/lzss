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

    constexpr HuffmanTable<Token>&& table() && {}

   private:
    struct Node {
        using NodeOrLeaf = std::variant<std::unique_ptr<Node>, Token>;

        NodeOrLeaf left;
        NodeOrLeaf right;
        Node* parent = nullptr;
    };

    using NodeOrLeaf = Node::NodeOrLeaf;

    Map<CountTp, std::vector<NodeOrLeaf>> work_table_;

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

    constexpr void ProcessWorkTable() {
        // Map sort elements in ascending order
        while (work_table_.size() > 1) {
            auto iter = work_table_.begin();
            if (iter->second.size() > 1) {
                EmplaceEquivariantSupernode(iter);
            } else {
            }
        }
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
