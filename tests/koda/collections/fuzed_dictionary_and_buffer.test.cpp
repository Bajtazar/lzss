#include <koda/collections/fuzed_dictionary_and_buffer.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <random>
#include <ranges>

static constexpr size_t kDictSize = 32;
static constexpr std::array<uint8_t, 4> kBuffer{0x12, 0x43, 0x55, 0x54};
static constexpr std::basic_string_view<uint8_t> kBufferView{kBuffer};

template <std::integral... Ints>
static std::basic_string<uint8_t> AsString(Ints... chars) {
    std::vector<uint8_t> vector{
        static_cast<uint8_t>(std::forward<Ints>(chars))...};
    std::basic_string<uint8_t> string;
    string.resize(vector.size());
    std::memcpy(string.data(), vector.data(), vector.size());
    return string;
}

TEST(FuzedDictionaryAndBuffer, Creation) {
    koda::FusedDictionaryAndBuffer dict{kDictSize, kBufferView};

    ASSERT_EQ(dict.buffer_size(), kBuffer.size());
    ASSERT_EQ(dict.max_buffer_size(), kBuffer.size());
    ASSERT_EQ(dict.dictionary_size(), 0);
    ASSERT_EQ(dict.max_dictionary_size(), kDictSize);

    ASSERT_EQ(dict.get_buffer(), kBufferView);
    ASSERT_EQ(dict.get_oldest_dictionary_full_match(), kBufferView);
}

TEST(FuzedDictionaryAndBuffer, SimpleBufferAccomodation) {
    koda::FusedDictionaryAndBuffer dict{kDictSize, kBufferView};

    ASSERT_EQ(dict.dictionary_size(), 0);

    dict.AddSymbolToBuffer(0x78);

    ASSERT_EQ(dict.get_buffer(), AsString(0x43, 0x55, 0x54, 0x78));
    ASSERT_EQ(dict.get_oldest_dictionary_full_match(), kBufferView);
    ASSERT_EQ(dict.dictionary_size(), 1);

    dict.AddSymbolToBuffer(0x54);

    ASSERT_EQ(dict.get_buffer(), AsString(0x55, 0x54, 0x78, 0x54));
    ASSERT_EQ(dict.get_oldest_dictionary_full_match(), kBufferView);
    ASSERT_EQ(dict.dictionary_size(), 2);

    dict.AddSymbolToBuffer(0x67);

    ASSERT_EQ(dict.get_buffer(), AsString(0x54, 0x78, 0x54, 0x67));
    ASSERT_EQ(dict.get_oldest_dictionary_full_match(), kBufferView);
    ASSERT_EQ(dict.dictionary_size(), 3);

    dict.AddSymbolToBuffer(0x93);

    ASSERT_EQ(dict.get_buffer(), AsString(0x78, 0x54, 0x67, 0x93));
    ASSERT_EQ(dict.get_oldest_dictionary_full_match(), kBufferView);
    ASSERT_EQ(dict.dictionary_size(), 4);

    dict.AddSymbolToBuffer(0x66);

    ASSERT_EQ(dict.get_buffer(), AsString(0x54, 0x67, 0x93, 0x66));
    ASSERT_EQ(dict.get_oldest_dictionary_full_match(), kBufferView);
    ASSERT_EQ(dict.dictionary_size(), 5);
}

static std::vector<uint8_t> GeneratePseudoNumberSequence(size_t length) {
    std::mt19937 engine;
    std::uniform_int_distribution<uint8_t> distibution{0, 255};

    std::vector<uint8_t> result;
    result.resize(length);

    std::ranges::for_each(
        result, [&](auto& element) { element = distibution(engine); });

    return result;
}

TEST(FuzedDictionaryAndBuffer, LongRunBufferAndDict) {
    koda::FusedDictionaryAndBuffer dict{kDictSize, kBufferView};
    // stress test
    auto sequence = GeneratePseudoNumberSequence(10'000);
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
