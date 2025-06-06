#include <koda/collections/fuzed_dictionary_and_buffer.hpp>
#include <koda/tests/tests.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <random>
#include <ranges>

#include "viewable_vector.hpp"

using namespace koda::tests;

static constexpr size_t kDictSize = 32;
static constexpr std::array<uint8_t, 4> kBuffer{0x12, 0x43, 0x55, 0x54};
static constexpr std::basic_string_view<uint8_t> kBufferView{kBuffer};
static constexpr size_t kRepeatitons = 10'000;

template <std::integral... Ints>
static constexpr ViewableVector<uint8_t> AsString(Ints... chars) {
    return ViewableVector<uint8_t>{
        static_cast<uint8_t>(std::forward<Ints>(chars))...};
}

BeginConstexprTest(FuzedDictionaryAndBufferTest, Creation) {
    koda::FusedDictionaryAndBuffer dict{kDictSize, kBufferView};

    ConstexprEqual(dict.buffer_size(), kBuffer.size());
    ConstexprEqual(dict.max_buffer_size(), kBuffer.size());
    ConstexprEqual(dict.dictionary_size(), 0);
    ConstexprEqual(dict.max_dictionary_size(), kDictSize);

    ConstexprEqual(dict.get_buffer(), kBufferView);
    ConstexprEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
}
EndConstexprTest(FuzedDictionaryAndBufferTest, Creation);

BeginConstexprTest(FuzedDictionaryAndBufferTest, SimpleBufferAccomodation) {
    koda::FusedDictionaryAndBuffer dict{kDictSize, kBufferView};

    ConstexprEqual(dict.dictionary_size(), 0);

    dict.AddSymbolToBuffer(0x78);

    ConstexprEqual(dict.get_buffer(), AsString(0x43, 0x55, 0x54, 0x78));
    ConstexprEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprEqual(dict.dictionary_size(), 1);

    dict.AddSymbolToBuffer(0x54);

    ConstexprEqual(dict.get_buffer(), AsString(0x55, 0x54, 0x78, 0x54));
    ConstexprEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprEqual(dict.dictionary_size(), 2);

    dict.AddSymbolToBuffer(0x67);

    ConstexprEqual(dict.get_buffer(), AsString(0x54, 0x78, 0x54, 0x67));
    ConstexprEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprEqual(dict.dictionary_size(), 3);

    dict.AddSymbolToBuffer(0x93);

    ConstexprEqual(dict.get_buffer(), AsString(0x78, 0x54, 0x67, 0x93));
    ConstexprEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprEqual(dict.dictionary_size(), 4);

    dict.AddSymbolToBuffer(0x66);

    ConstexprEqual(dict.get_buffer(), AsString(0x54, 0x67, 0x93, 0x66));
    ConstexprEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprEqual(dict.dictionary_size(), 5);
}
EndConstexprTest(FuzedDictionaryAndBufferTest, SimpleBufferAccomodation);

static std::vector<uint8_t> GeneratePseudoNumberSequence(size_t length) {
    std::mt19937 engine;
    std::uniform_int_distribution<uint8_t> distibution{0, 255};

    std::vector<uint8_t> result;
    result.resize(length);

    std::ranges::for_each(
        result, [&](auto& element) { element = distibution(engine); });

    return result;
}

TEST(FuzedDictionaryAndBufferTest, LongRunBufferAndDict) {
    koda::FusedDictionaryAndBuffer dict{kDictSize, kBufferView};
    // stress test
    auto sequence = GeneratePseudoNumberSequence(kRepeatitons);
    // Override buffer
    for (auto const& elem : sequence | std::views::take(kBufferView.size())) {
        dict.AddSymbolToBuffer(elem);
    }

    auto buff_iter = 0;
    for (auto const& elem : sequence | std::views::drop(kBufferView.size()) |
                                std::views::take(kDictSize)) {
        ASSERT_EQ(dict.get_buffer(),
                  AsString(sequence[buff_iter], sequence[buff_iter + 1],
                           sequence[buff_iter + 2], sequence[buff_iter + 3]));
        ++buff_iter;
        dict.AddSymbolToBuffer(elem);
    }
    // Now N-last element of dict are N-last elements of the sequence!
    auto iter = 0;
    for (auto const& elem :
         sequence | std::views::drop(kDictSize + kBufferView.size())) {
        ASSERT_EQ(dict.get_buffer(),
                  AsString(sequence[buff_iter], sequence[buff_iter + 1],
                           sequence[buff_iter + 2], sequence[buff_iter + 3]));
        ASSERT_EQ(dict.get_oldest_dictionary_full_match(),
                  AsString(sequence[iter], sequence[iter + 1],
                           sequence[iter + 2], sequence[iter + 3]));
        ++iter;
        ++buff_iter;
        dict.AddSymbolToBuffer(elem);
    }
}

TEST(FuzedDictionaryAndBufferTest, AddEndSymbolTests) {
    koda::FusedDictionaryAndBuffer dict{kDictSize, kBufferView};
    // stress test
    auto sequence =
        GeneratePseudoNumberSequence(kDictSize + kBufferView.size());
    // Override buffer and dict
    for (auto const& elem :
         sequence | std::views::take(kDictSize + kBufferView.size())) {
        dict.AddSymbolToBuffer(elem);
    }
    // Now N-last element of dict are N-last elements of the sequence!
    dict.AddEndSymbolToBuffer();

    ASSERT_EQ(dict.get_oldest_dictionary_full_match(),
              AsString(sequence[1], sequence[2], sequence[3], sequence[4]));
    ASSERT_EQ(dict.get_buffer(),
              AsString(sequence[kDictSize + 1], sequence[kDictSize + 2],
                       sequence[kDictSize + 3]));
    ASSERT_EQ(dict.dictionary_size(), dict.max_dictionary_size());
    ASSERT_EQ(dict.buffer_size(), dict.max_buffer_size() - 1);

    dict.AddEndSymbolToBuffer();

    ASSERT_EQ(dict.get_oldest_dictionary_full_match(),
              AsString(sequence[2], sequence[3], sequence[4], sequence[5]));
    ASSERT_EQ(dict.get_buffer(),
              AsString(sequence[kDictSize + 2], sequence[kDictSize + 3]));
    ASSERT_EQ(dict.dictionary_size(), dict.max_dictionary_size());
    ASSERT_EQ(dict.buffer_size(), dict.max_buffer_size() - 2);

    dict.AddEndSymbolToBuffer();

    ASSERT_EQ(dict.get_oldest_dictionary_full_match(),
              AsString(sequence[3], sequence[4], sequence[5], sequence[6]));
    ASSERT_EQ(dict.get_buffer(), AsString(sequence[kDictSize + 3]));
    ASSERT_EQ(dict.dictionary_size(), dict.max_dictionary_size());
    ASSERT_EQ(dict.buffer_size(), dict.max_buffer_size() - 3);

    dict.AddEndSymbolToBuffer();

    ASSERT_EQ(dict.get_oldest_dictionary_full_match(),
              AsString(sequence[4], sequence[5], sequence[6], sequence[7]));
    ASSERT_EQ(dict.get_buffer(), std::basic_string_view<uint8_t>{});
    ASSERT_EQ(dict.dictionary_size(), dict.max_dictionary_size());
    ASSERT_EQ(dict.buffer_size(), 0);
}
