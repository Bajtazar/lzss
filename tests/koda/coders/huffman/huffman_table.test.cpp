#include <koda/coders/huffman/huffman_table.hpp>
#include <koda/tests/tests.hpp>


BeginConstexprTest(HuffmanTable, A) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    koda::HuffmanTable<uint32_t> kExpected = {
        HuffmanEntry{0, std::vector<bool>{0}}
    };

    koda::Map<uint32_t, size_t> res;
    res.Emplace(5, 32);
    res.Emplace(1, 4);
    res.Emplace(0, 54);
    res.Emplace(32, 16);
    res.Emplace(43, 16);
    res.Emplace(16, 22);

    auto ves = koda::MakeHuffmanTable(res);
}
EndConstexprTest;
