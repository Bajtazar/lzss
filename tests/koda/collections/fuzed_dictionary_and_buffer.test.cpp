#include <koda/collections/fuzed_dictionary_and_buffer.hpp>

#include <gtest/gtest.h>

#include <array>

static constexpr size_t kDictSize = 32;
static constexpr std::array<uint8_t, 4> kBuffer{0x12, 0x43, 0x55, 0x54};
static constexpr std::basic_string_view<uint8_t> kBufferView{kBuffer};

TEST(FuzedDictionaryAndBuffer, Creation) {
    koda::FusedDictionaryAndBuffer dict{kDictSize, kBufferView};

    ASSERT_EQ(dict.buffer_size(), kBuffer.size());
    ASSERT_EQ(dict.max_buffer_size(), kBuffer.size());
    ASSERT_EQ(dict.dictionary_size(), 0);
    ASSERT_EQ(dict.max_dictionary_size(), kDictSize);

    ASSERT_EQ(dict.get_buffer(), kBufferView);
    ASSERT_EQ(dict.get_oldest_dictionary_full_match(),
              std::basic_string_view<uint8_t>{});
}
