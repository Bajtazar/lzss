#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

#include <iterator>
#include <sstream>
#include <string>
#include <vector>

TEST(LittleEndianInputBitIterTest, AppendBits) {
    std::vector<uint8_t> bytes = {0b101011};
    koda::LittleEndianInputBitIter iter{bytes.cbegin()};

    ASSERT_EQ(iter.Position(), 0);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 1);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 2);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 3);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 4);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 5);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 6);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 7);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 0);

    ASSERT_NE(iter, koda::LittleEndianInputBitIter{bytes.cbegin()});
}

TEST(LittleEndianInputBitIterTest, Skip) {
    std::vector<uint8_t> bytes = {0b101011, 0b1101};
    koda::LittleEndianInputBitIter iter{bytes.cbegin()};

    ASSERT_EQ(iter.Position(), 0);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 1);
    ASSERT_TRUE(*iter++);
    iter.SkipToNextByte();
    ASSERT_EQ(iter.Position(), 0);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 1);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 2);

    ASSERT_EQ(iter, koda::LittleEndianInputBitIter{std::next(bytes.cbegin())});
}

TEST(LittleEndianInputBitIterTest, ReadByte) {
    std::vector<uint8_t> bytes = {0b101011, 0b1101};
    koda::LittleEndianInputBitIter iter{bytes.cbegin()};

    ASSERT_EQ(iter.Position(), 0);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 1);
    ASSERT_TRUE(*iter++);
    iter.SkipToNextByte();
    ASSERT_EQ(iter.ReadByte(), std::byte{0b1101});

    ASSERT_EQ(iter, koda::LittleEndianInputBitIter{bytes.cend()});
}

TEST(BigEndianInputBitIterTest, AppendBits) {
    std::vector<uint8_t> bytes = {0b101011};
    koda::BigEndianInputBitIter iter{bytes.cbegin()};

    ASSERT_EQ(iter.Position(), 7);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 6);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 5);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 4);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 3);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 2);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 1);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 0);
    ASSERT_TRUE(*iter++);
    ASSERT_EQ(iter.Position(), 7);

    ASSERT_NE(iter, koda::BigEndianInputBitIter{bytes.cbegin()});
}

TEST(BigEndianInputBitIterTest, Skip) {
    std::vector<uint8_t> bytes = {0b101011, 0b1101};
    koda::BigEndianInputBitIter iter{bytes.cbegin()};

    ASSERT_EQ(iter.Position(), 7);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 6);
    ASSERT_FALSE(*iter++);
    iter.SkipToNextByte();
    ASSERT_EQ(iter.Position(), 7);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 6);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 5);

    ASSERT_EQ(iter, koda::BigEndianInputBitIter{std::next(bytes.cbegin())});
}

TEST(BigEndianInputBitIterTest, ReadByte) {
    std::vector<uint8_t> bytes = {0b101011, 0b1101};
    koda::BigEndianInputBitIter iter{bytes.cbegin()};

    ASSERT_EQ(iter.Position(), 7);
    ASSERT_FALSE(*iter++);
    ASSERT_EQ(iter.Position(), 6);
    ASSERT_FALSE(*iter++);
    iter.SkipToNextByte();
    ASSERT_EQ(iter.ReadByte(), std::byte{0b1101});

    ASSERT_EQ(iter, koda::BigEndianInputBitIter{bytes.cend()});
}

TEST(LittleEndianOutputBitIterTest, AppendBits) {
    std::vector<uint8_t> result;
    koda::LittleEndianOutputBitIter iter{std::back_inserter(result)};

    ASSERT_EQ(iter.Position(), 0);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 1);
    *iter++ = 0;
    ASSERT_EQ(iter.Position(), 2);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 3);
    *iter++ = 0;
    ASSERT_EQ(iter.Position(), 4);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 5);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 6);
    *iter++ = 0;
    ASSERT_EQ(iter.Position(), 7);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 0);

    ASSERT_FALSE(result.empty());
    ASSERT_EQ(result.front(), 0b10110101);
}

TEST(LittleEndianOutputBitIterTest, Skip) {
    std::vector<uint8_t> result;
    koda::LittleEndianOutputBitIter iter{std::back_inserter(result)};

    ASSERT_EQ(iter.Position(), 0);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 1);
    *iter++ = 0;
    ASSERT_EQ(iter.Position(), 2);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 3);
    iter.SkipToNextByte();
    ASSERT_EQ(iter.Position(), 0);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 1);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 2);
    iter.SkipToNextByte();
    ASSERT_EQ(iter.Position(), 0);

    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result[0], 0b101);
    ASSERT_EQ(result[1], 0b11);
}

TEST(LittleEndianOutputBitIterTest, SaveByte) {
    std::vector<uint8_t> result;
    koda::LittleEndianOutputBitIter iter{std::back_inserter(result)};

    ASSERT_EQ(iter.Position(), 0);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 1);
    *iter++ = 1;
    ASSERT_EQ(iter.Position(), 2);
    iter.SaveByte(0b1101);
    ASSERT_EQ(iter.Position(), 0);
    iter.SaveByte(0b1001);
    ASSERT_EQ(iter.Position(), 0);

    ASSERT_EQ(result.size(), 3);
    ASSERT_EQ(result[0], 0b11);
    ASSERT_EQ(result[1], 0b1101);
    ASSERT_EQ(result[2], 0b1001);
}
