#include <koda/tests/tests.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <iterator>
#include <sstream>
#include <string>
#include <vector>

BeginConstexprTest(LittleEndianInputBitIterTest, AppendBits) {
    std::vector<uint8_t> bytes = {0b101011};
    koda::LittleEndianInputBitIter iter{bytes.cbegin()};

    ConstexprAssertEqual(iter.Position(), 0);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 1);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 2);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 3);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 4);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 5);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 6);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 7);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 0);

    ConstexprAssertNotEqual(iter,
                            koda::LittleEndianInputBitIter{bytes.cbegin()});
}
EndConstexprTest(LittleEndianInputBitIterTest, AppendBits);

BeginConstexprTest(LittleEndianInputBitIterTest, Skip) {
    std::vector<uint8_t> bytes = {0b101011, 0b1101};
    koda::LittleEndianInputBitIter iter{bytes.cbegin()};

    ConstexprAssertEqual(iter.Position(), 0);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 1);
    ConstexprAssertTrue(*iter++);
    iter.SkipToNextByte();
    ConstexprAssertEqual(iter.Position(), 0);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 1);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 2);

    ConstexprAssertEqual(
        iter, koda::LittleEndianInputBitIter{std::next(bytes.cbegin())});
}
EndConstexprTest(LittleEndianInputBitIterTest, Skip);

BeginConstexprTest(LittleEndianInputBitIterTest, ReadByte) {
    std::vector<uint8_t> bytes = {0b101011, 0b1101};
    koda::LittleEndianInputBitIter iter{bytes.cbegin()};

    ConstexprAssertEqual(iter.Position(), 0);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 1);
    ConstexprAssertTrue(*iter++);
    iter.SkipToNextByte();
    ConstexprAssertEqual(iter.ReadByte(), std::byte{0b1101});

    ConstexprAssertEqual(iter, koda::LittleEndianInputBitIter{bytes.cend()});
}
EndConstexprTest(LittleEndianInputBitIterTest, ReadByte);

BeginConstexprTest(BigEndianInputBitIterTest, AppendBits) {
    std::vector<uint8_t> bytes = {0b101011};
    koda::BigEndianInputBitIter iter{bytes.cbegin()};

    ConstexprAssertEqual(iter.Position(), 7);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 6);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 5);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 4);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 3);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 2);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 1);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 0);
    ConstexprAssertTrue(*iter++);
    ConstexprAssertEqual(iter.Position(), 7);

    ConstexprAssertNotEqual(iter, koda::BigEndianInputBitIter{bytes.cbegin()});
}
EndConstexprTest(BigEndianInputBitIterTest, AppendBits);

BeginConstexprTest(BigEndianInputBitIterTest, Skip) {
    std::vector<uint8_t> bytes = {0b101011, 0b1101};
    koda::BigEndianInputBitIter iter{bytes.cbegin()};

    ConstexprAssertEqual(iter.Position(), 7);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 6);
    ConstexprAssertFalse(*iter++);
    iter.SkipToNextByte();
    ConstexprAssertEqual(iter.Position(), 7);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 6);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 5);

    ConstexprAssertEqual(
        iter, koda::BigEndianInputBitIter{std::next(bytes.cbegin())});
}
EndConstexprTest(BigEndianInputBitIterTest, Skip);

BeginConstexprTest(BigEndianInputBitIterTest, ReadByte) {
    std::vector<uint8_t> bytes = {0b101011, 0b1101};
    koda::BigEndianInputBitIter iter{bytes.cbegin()};

    ConstexprAssertEqual(iter.Position(), 7);
    ConstexprAssertFalse(*iter++);
    ConstexprAssertEqual(iter.Position(), 6);
    ConstexprAssertFalse(*iter++);
    iter.SkipToNextByte();
    ConstexprAssertEqual(iter.ReadByte(), std::byte{0b1101});

    ConstexprAssertEqual(iter, koda::BigEndianInputBitIter{bytes.cend()});
}
EndConstexprTest(BigEndianInputBitIterTest, ReadByte);

BeginConstexprTest(LittleEndianOutputBitIterTest, AppendBits) {
    std::vector<uint8_t> result;
    koda::LittleEndianOutputBitIter iter{std::back_inserter(result)};

    ConstexprAssertEqual(iter.Position(), 0);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 1);
    *iter++ = 0;
    ConstexprAssertEqual(iter.Position(), 2);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 3);
    *iter++ = 0;
    ConstexprAssertEqual(iter.Position(), 4);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 5);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 6);
    *iter++ = 0;
    ConstexprAssertEqual(iter.Position(), 7);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 0);

    ConstexprAssertFalse(result.empty());
    ConstexprAssertEqual(result.front(), 0b10110101);
}
EndConstexprTest(LittleEndianOutputBitIterTest, AppendBits);

BeginConstexprTest(LittleEndianOutputBitIterTest, Skip) {
    std::vector<uint8_t> result;
    koda::LittleEndianOutputBitIter iter{std::back_inserter(result)};

    ConstexprAssertEqual(iter.Position(), 0);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 1);
    *iter++ = 0;
    ConstexprAssertEqual(iter.Position(), 2);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 3);
    iter.SkipToNextByte();
    ConstexprAssertEqual(iter.Position(), 0);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 1);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 2);
    iter.SkipToNextByte();
    ConstexprAssertEqual(iter.Position(), 0);

    ConstexprAssertEqual(result.size(), 2);
    ConstexprAssertEqual(result[0], 0b101);
    ConstexprAssertEqual(result[1], 0b11);
}
EndConstexprTest(LittleEndianOutputBitIterTest, Skip);

BeginConstexprTest(LittleEndianOutputBitIterTest, SaveByte) {
    std::vector<uint8_t> result;
    koda::LittleEndianOutputBitIter iter{std::back_inserter(result)};

    ConstexprAssertEqual(iter.Position(), 0);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 1);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 2);
    iter.SaveByte(0b1101);
    ConstexprAssertEqual(iter.Position(), 0);
    iter.SaveByte(0b1001);
    ConstexprAssertEqual(iter.Position(), 0);

    ConstexprAssertEqual(result.size(), 3);
    ConstexprAssertEqual(result[0], 0b11);
    ConstexprAssertEqual(result[1], 0b1101);
    ConstexprAssertEqual(result[2], 0b1001);
}
EndConstexprTest(LittleEndianOutputBitIterTest, SaveByte);

BeginConstexprTest(BigEndianOutputBitIterIterTest, AppendBits) {
    std::vector<uint8_t> result;
    koda::BigEndianOutputBitIter iter{std::back_inserter(result)};

    ConstexprAssertEqual(iter.Position(), 7);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 6);
    *iter++ = 0;
    ConstexprAssertEqual(iter.Position(), 5);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 4);
    *iter++ = 0;
    ConstexprAssertEqual(iter.Position(), 3);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 2);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 1);
    *iter++ = 0;
    ConstexprAssertEqual(iter.Position(), 0);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 7);

    ConstexprAssertFalse(result.empty());
    ConstexprAssertEqual(result.front(), 0b10101101);
}
EndConstexprTest(BigEndianOutputBitIterIterTest, AppendBits);

BeginConstexprTest(BigEndianOutputBitIterIterTest, Skip) {
    std::vector<uint8_t> result;
    koda::BigEndianOutputBitIter iter{std::back_inserter(result)};

    ConstexprAssertEqual(iter.Position(), 7);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 6);
    *iter++ = 0;
    ConstexprAssertEqual(iter.Position(), 5);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 4);
    iter.SkipToNextByte();
    ConstexprAssertEqual(iter.Position(), 7);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 6);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 5);
    iter.SkipToNextByte();
    ConstexprAssertEqual(iter.Position(), 7);

    ConstexprAssertEqual(result.size(), 2);
    ConstexprAssertEqual(result[0], 0b10100000);
    ConstexprAssertEqual(result[1], 0b11000000);
}
EndConstexprTest(BigEndianOutputBitIterIterTest, Skip);

BeginConstexprTest(BigEndianOutputBitIterIterTest, SaveByte) {
    std::vector<uint8_t> result;
    koda::BigEndianOutputBitIter iter{std::back_inserter(result)};

    ConstexprAssertEqual(iter.Position(), 7);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 6);
    *iter++ = 1;
    ConstexprAssertEqual(iter.Position(), 5);
    iter.SaveByte(0b1101);
    ConstexprAssertEqual(iter.Position(), 7);
    iter.SaveByte(0b1001);
    ConstexprAssertEqual(iter.Position(), 7);

    ConstexprAssertEqual(result.size(), 3);
    ConstexprAssertEqual(result[0], 0b11000000);
    ConstexprAssertEqual(result[1], 0b1101);
    ConstexprAssertEqual(result[2], 0b1001);
}
EndConstexprTest(BigEndianOutputBitIterIterTest, SaveByte);
