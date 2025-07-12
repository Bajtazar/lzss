#include <koda/coders/huffman/huffman_table.hpp>

#include <cassert>
#include <vector>

template <typename Token>
constexpr std::vector<bool> ConcatenateSymbols(
    const koda::HuffmanTable<Token>& table, const auto& tokens) {
    std::vector<bool> result;
    for (const auto& token : tokens) {
        auto iter = table.Find(token);
        assert(iter != table.end());
        result.insert_range(result.end(), iter->second);
    }
    return result;
}
