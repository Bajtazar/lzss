#pragma once

#include <koda/collections/map.hpp>

#include <cinttypes>
#include <concepts>
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

}  // namespace koda

#include <koda/coders/huffman/huffman_table.tpp>
