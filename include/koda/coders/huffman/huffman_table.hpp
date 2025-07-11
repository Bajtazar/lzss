#pragma once

#include <koda/collections/map.hpp>

#include <cinttypes>
#include <concepts>
#include <vector>

namespace koda {

template <std::integral Token>
using HuffmanTable = Map<Token, std::vector<bool>>;

template <std::integral Token, std::integral CountTp>
[[nodiscard]] constexpr HuffmanTable<Token> MakeHuffmanTable(
    const Map<Token, CountTp>& count);

}  // namespace koda

#include <koda/coders/huffman/huffman_table.tpp>
