#include <koda/coders/huffman/huffman_table.hpp>
#include <koda/tests/tests.hpp>

BeginConstexprTest(HuffmanTable, FirstScenario) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    const koda::HuffmanTable<uint32_t> kExpected = {
        HuffmanEntry{0, std::vector<bool>{0}},
        HuffmanEntry{1, std::vector<bool>{1, 0, 1, 0}},
        HuffmanEntry{5, std::vector<bool>{1, 1, 1}},
        HuffmanEntry{16, std::vector<bool>{1, 1, 0}},
        HuffmanEntry{32, std::vector<bool>{1, 0, 0}},
        HuffmanEntry{43, std::vector<bool>{1, 0, 1, 1}}};

    koda::Map<uint32_t, size_t> res;
    res.Emplace(5, 32);
    res.Emplace(1, 4);
    res.Emplace(0, 54);
    res.Emplace(32, 16);
    res.Emplace(43, 16);
    res.Emplace(16, 22);

    auto table = koda::MakeHuffmanTable(res);

    ConstexprAssertEqual(table, kExpected);
}
EndConstexprTest;
