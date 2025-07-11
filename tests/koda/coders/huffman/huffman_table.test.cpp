#include <koda/coders/huffman/huffman_table.hpp>
#include <koda/tests/tests.hpp>

BeginConstexprTest(HuffmanTable, FirstScenario) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    // One of the valid results, returned by the algorithm and verified by hand
    const koda::HuffmanTable<uint32_t> kExpected = {
        HuffmanEntry{0, std::vector<bool>{0}},
        HuffmanEntry{1, std::vector<bool>{1, 0, 1, 0}},
        HuffmanEntry{5, std::vector<bool>{1, 1, 1}},
        HuffmanEntry{16, std::vector<bool>{1, 1, 0}},
        HuffmanEntry{32, std::vector<bool>{1, 0, 0}},
        HuffmanEntry{43, std::vector<bool>{1, 0, 1, 1}}};

    koda::Map<uint32_t, size_t> counts = {{5, 32},  {1, 4},   {0, 54},
                                          {32, 16}, {43, 16}, {16, 22}};

    auto table = koda::MakeHuffmanTable(counts);

    ConstexprAssertEqual(table, kExpected);
}
EndConstexprTest;
