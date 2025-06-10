#include <koda/collections/fuzed_dictionary_and_buffer.hpp>
#include <koda/tests/tests.hpp>
#include <koda/tests/viewable_vector.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <ranges>

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
    koda::FusedDictionaryAndBuffer<uint8_t> dict{kDictSize, kBufferView};

    ConstexprAssertEqual(dict.buffer_size(), kBuffer.size());
    ConstexprAssertEqual(dict.max_buffer_size(), kBuffer.size());
    ConstexprAssertEqual(dict.dictionary_size(), 0);
    ConstexprAssertEqual(dict.max_dictionary_size(), kDictSize);

    ConstexprAssertEqual(dict.get_buffer(), kBufferView);
    ConstexprAssertEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
}
EndConstexprTest;

BeginConstexprTest(FuzedDictionaryAndBufferTest, SimpleBufferAccomodation) {
    koda::FusedDictionaryAndBuffer dict{kDictSize, kBufferView};

    ConstexprAssertEqual(dict.dictionary_size(), 0);

    dict.AddSymbolToBuffer(0x78);

    ConstexprAssertEqual(dict.get_buffer(), AsString(0x43, 0x55, 0x54, 0x78));
    ConstexprAssertEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprAssertEqual(dict.dictionary_size(), 1);

    dict.AddSymbolToBuffer(0x54);

    ConstexprAssertEqual(dict.get_buffer(), AsString(0x55, 0x54, 0x78, 0x54));
    ConstexprAssertEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprAssertEqual(dict.dictionary_size(), 2);

    dict.AddSymbolToBuffer(0x67);

    ConstexprAssertEqual(dict.get_buffer(), AsString(0x54, 0x78, 0x54, 0x67));
    ConstexprAssertEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprAssertEqual(dict.dictionary_size(), 3);

    dict.AddSymbolToBuffer(0x93);

    ConstexprAssertEqual(dict.get_buffer(), AsString(0x78, 0x54, 0x67, 0x93));
    ConstexprAssertEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprAssertEqual(dict.dictionary_size(), 4);

    dict.AddSymbolToBuffer(0x66);

    ConstexprAssertEqual(dict.get_buffer(), AsString(0x54, 0x67, 0x93, 0x66));
    ConstexprAssertEqual(dict.get_oldest_dictionary_full_match(), kBufferView);
    ConstexprAssertEqual(dict.dictionary_size(), 5);
}
EndConstexprTest;

static constexpr std::vector<uint8_t> GeneratePseudoNumberSequence(
    size_t length) {
    static constexpr unsigned kAlpha = 16807;
    static constexpr unsigned kModulo = 2147483647;
    size_t value = 43948390;

    std::vector<uint8_t> result;
    result.resize(length);

    std::ranges::for_each(result, [&](auto& element) {
        element = static_cast<uint8_t>(value);
        value = (kAlpha * value) % kModulo;
    });

    return result;
}

BeginConstexprTest(FuzedDictionaryAndBufferTest, LongRunBufferAndDict) {
    koda::FusedDictionaryAndBuffer<uint8_t> dict{kDictSize, kBufferView};
    // stress test
    auto sequence = GeneratePseudoNumberSequence(kRepeatitons);
    // Override buffer
    for (auto const& elem : sequence | std::views::take(kBufferView.size())) {
        dict.AddSymbolToBuffer(elem);
    }

    auto buff_iter = 0;
    for (auto const& elem : sequence | std::views::drop(kBufferView.size()) |
                                std::views::take(kDictSize)) {
        ConstexprAssertEqual(
            dict.get_buffer(),
            AsString(sequence[buff_iter], sequence[buff_iter + 1],
                     sequence[buff_iter + 2], sequence[buff_iter + 3]));
        ++buff_iter;
        dict.AddSymbolToBuffer(elem);
    }
    // Now N-last element of dict are N-last elements of the sequence!
    auto iter = 0;
    for (auto const& elem :
         sequence | std::views::drop(kDictSize + kBufferView.size())) {
        ConstexprAssertEqual(
            dict.get_buffer(),
            AsString(sequence[buff_iter], sequence[buff_iter + 1],
                     sequence[buff_iter + 2], sequence[buff_iter + 3]));
        ConstexprAssertEqual(dict.get_oldest_dictionary_full_match(),
                             AsString(sequence[iter], sequence[iter + 1],
                                      sequence[iter + 2], sequence[iter + 3]));
        ++iter;
        ++buff_iter;
        dict.AddSymbolToBuffer(elem);
    }
}
EndConstexprTest;

BeginConstexprTest(FuzedDictionaryAndBufferTest, AddEndSymbolTests) {
    koda::FusedDictionaryAndBuffer<uint8_t> dict{kDictSize, kBufferView};
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

    ConstexprAssertEqual(
        dict.get_oldest_dictionary_full_match(),
        AsString(sequence[1], sequence[2], sequence[3], sequence[4]));
    ConstexprAssertEqual(dict.get_buffer(), AsString(sequence[kDictSize + 1],
                                                     sequence[kDictSize + 2],
                                                     sequence[kDictSize + 3]));
    ConstexprAssertEqual(dict.dictionary_size(), dict.max_dictionary_size());
    ConstexprAssertEqual(dict.buffer_size(), dict.max_buffer_size() - 1);

    dict.AddEndSymbolToBuffer();

    ConstexprAssertEqual(
        dict.get_oldest_dictionary_full_match(),
        AsString(sequence[2], sequence[3], sequence[4], sequence[5]));
    ConstexprAssertEqual(dict.get_buffer(), AsString(sequence[kDictSize + 2],
                                                     sequence[kDictSize + 3]));
    ConstexprAssertEqual(dict.dictionary_size(), dict.max_dictionary_size());
    ConstexprAssertEqual(dict.buffer_size(), dict.max_buffer_size() - 2);

    dict.AddEndSymbolToBuffer();

    ConstexprAssertEqual(
        dict.get_oldest_dictionary_full_match(),
        AsString(sequence[3], sequence[4], sequence[5], sequence[6]));
    ConstexprAssertEqual(dict.get_buffer(), AsString(sequence[kDictSize + 3]));
    ConstexprAssertEqual(dict.dictionary_size(), dict.max_dictionary_size());
    ConstexprAssertEqual(dict.buffer_size(), dict.max_buffer_size() - 3);

    dict.AddEndSymbolToBuffer();

    ConstexprAssertEqual(
        dict.get_oldest_dictionary_full_match(),
        AsString(sequence[4], sequence[5], sequence[6], sequence[7]));
    ConstexprAssertEqual(dict.get_buffer(), std::basic_string_view<uint8_t>{});
    ConstexprAssertEqual(dict.dictionary_size(), dict.max_dictionary_size());
    ConstexprAssertEqual(dict.buffer_size(), 0);
}
EndConstexprTest;

BeginConstexprTest(FuzedDictionaryAndBufferTest, PositionGetterStraight) {
    static constexpr uint16_t kBufferLen = 4;

    koda::FusedDictionaryAndBuffer<uint16_t> dict{
        kDictSize, koda::FusedDictionaryAndBuffer<uint16_t>::SequenceView{
                       std::views::iota(uint16_t{0}, kBufferLen) |
                       std::ranges::to<std::vector>()}};

    for (uint16_t i = kBufferLen; i < kDictSize; ++i) {
        dict.AddSymbolToBuffer(i);
    }

    for (uint16_t len = 1; len <= kBufferLen; ++len) {
        for (uint16_t pos = 0; pos < (kDictSize - kBufferLen); ++pos) {
            const auto expected =
                std::views::iota(pos, static_cast<uint16_t>(pos + len)) |
                std::ranges::to<std::vector>();
            auto sequence = dict.get_sequence_at_relative_pos(pos, len);
            ConstexprAssertEqual(expected, sequence);
        }
    }
}
EndConstexprTest;

BeginConstexprTest(FuzedDictionaryAndBufferTest, PositionGetterWrapped) {
    static constexpr uint16_t kBufferLen = 4;

    koda::FusedDictionaryAndBuffer<uint16_t> dict{
        kDictSize, koda::FusedDictionaryAndBuffer<uint16_t>::SequenceView{
                       std::views::iota(uint16_t{0}, kBufferLen) |
                       std::ranges::to<std::vector>()}};

    for (uint16_t i = kBufferLen; i <= kRepeatitons; ++i) {
        dict.AddSymbolToBuffer(i);
    }

    for (uint16_t pos = 0; pos < kDictSize; ++pos) {
        uint16_t length =
            pos + kBufferLen > kDictSize ? kDictSize - pos : kBufferLen;
        uint16_t symbol = pos + kRepeatitons - kDictSize - kBufferLen + 1;
        auto sequence = dict.get_sequence_at_relative_pos(pos, length);
        const auto expected =
            std::views::iota(symbol, static_cast<uint16_t>(symbol + length)) |
            std::ranges::to<std::vector>();

        ConstexprAssertEqual(expected, sequence);
    }
}
EndConstexprTest;
