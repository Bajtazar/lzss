#pragma once

#include <koda/coders/huffman/huffman_table.hpp>

#include <vector>

template <typename Token>
constexpr std::vector<bool> ConcatenateSymbols(
    const koda::HuffmanTable<Token>& table, const auto& tokens) {
    std::vector<bool> result;
    for (const auto& token : tokens) {
        result.insert_range(result.end(), table.At(token));
    }
    return result;
}
