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
            if (auto iter = work_table_.Find(occurences);
                iter != work_table_.end()) {
                iter->second.emplace_back(token);
            } else {
                work_table_.Emplace(occurences, token);
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
