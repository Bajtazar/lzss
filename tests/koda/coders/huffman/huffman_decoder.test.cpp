#include <koda/coders/huffman/huffman_decoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

static_assert(koda::Decoder<koda::HuffmanDecoder<uint8_t>, uint8_t>);

template <typename Token>
static constexpr std::vector<bool> ConcatenateSymbols(
    const koda::HuffmanTable<Token>& table, const auto& tokens) {
    std::vector<bool> result;
    for (const auto& token : tokens) {
        auto iter = table.Find(token);
        assert(iter != table.end());
        result.insert_range(result.end(), iter->second);
    }
    return result;
}
