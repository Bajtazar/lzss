#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

TEST(LittleEndianInputBitIterTest, AppendBits) {
    std::vector<uint8_t> bytes = {0b101011};
    koda::LittleEndianInputBitIter iter{bytes.cbegin()};

    ASSERT_TRUE(*iter++);
    ASSERT_TRUE(*iter++);
    ASSERT_FALSE(*iter++);
    ASSERT_TRUE(*iter++);
    ASSERT_FALSE(*iter++);
    ASSERT_TRUE(*iter++);
    ASSERT_FALSE(*iter++);
    ASSERT_FALSE(*iter++);

    ASSERT_NE(iter, koda::LittleEndianInputBitIter{bytes.cbegin()});
}
