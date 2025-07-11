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

BeginConstexprTest(HuffmanTable, ListScenario) {
    using HuffmanEntry = koda::HuffmanTable<uint32_t>::entry_type;

    const koda::HuffmanTable<uint32_t> kExpected = {
        HuffmanEntry{'t', std::vector<bool>{1}},
        HuffmanEntry{'r', std::vector<bool>{0, 1}},
        HuffmanEntry{'x', std::vector<bool>{0, 0, 1}},
        HuffmanEntry{'o', std::vector<bool>{0, 0, 0, 1}},
        HuffmanEntry{'e', std::vector<bool>{0, 0, 0, 0, 1}},
        HuffmanEntry{'a', std::vector<bool>{0, 0, 0, 0, 0}}};

    koda::Map<char, size_t> counts = {{'a', 1}, {'e', 2},  {'o', 4},
                                      {'x', 8}, {'r', 16}, {'t', 32}};

    auto table = koda::MakeHuffmanTable(counts);

    ConstexprAssertEqual(table, kExpected);
}
EndConstexprTest;

BeginConstexprTest(HuffmanTable, AllSameScenario) {
    using HuffmanEntry = koda::HuffmanTable<char>::entry_type;

    const koda::HuffmanTable<char> kExpected = {
        HuffmanEntry{'a', std::vector<bool>{1, 0, 1}},
        HuffmanEntry{'b', std::vector<bool>{1, 0, 0}},
        HuffmanEntry{'c', std::vector<bool>{1, 1, 1}},
        HuffmanEntry{'d', std::vector<bool>{1, 1, 0}},
        HuffmanEntry{'e', std::vector<bool>{0, 0, 1}},
        HuffmanEntry{'f', std::vector<bool>{0, 0, 0}},
        HuffmanEntry{'g', std::vector<bool>{0, 1, 1}},
        HuffmanEntry{'h', std::vector<bool>{0, 1, 0}}};

    koda::Map<char, size_t> counts = {{'a', 1}, {'b', 1}, {'c', 1}, {'d', 1},
                                      {'e', 1}, {'f', 1}, {'g', 1}, {'h', 1}};

    auto table = koda::MakeHuffmanTable(counts);

    ConstexprAssertEqual(table, kExpected);
}
EndConstexprTest;

BeginConstexprTest(HuffmanTable, OneElementScenario) {
    using HuffmanEntry = koda::HuffmanTable<char>::entry_type;

    const koda::HuffmanTable<char> kExpected = {
        HuffmanEntry{'a', std::vector<bool>{}}};

    koda::Map<char, size_t> counts = {{'a', 1}};

    auto table = koda::MakeHuffmanTable(counts);

    ConstexprAssertEqual(table, kExpected);
}
EndConstexprTest;
