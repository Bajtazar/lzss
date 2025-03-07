#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

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
