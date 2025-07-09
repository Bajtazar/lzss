#pragma once

#include <cinttypes>
#include <concepts>
#include <set>
#include <vector>

namespace koda {

template <std::integral Token>
struct HuffmanTableEntry {
    std::vector<bool> binary_symbol;
    Token token;
};

struct HuffmanTableEntryComparator {
    template <std::integral LToken, std::integral RToken>
    [[nodiscard]] static constexpr bool operator()(
        const HuffmanTableEntry<LToken>& left,
        const HuffmanTableEntry<RToken>& right) noexcept;
};

template <std::integral Token>
using HuffmanTable = std::set<HuffmanTableEntry<Token>>;

}  // namespace koda

#include <koda/coders/huffman/huffman_table.tpp>
