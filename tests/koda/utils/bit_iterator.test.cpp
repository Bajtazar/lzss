#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <iterator>
#include <sstream>
#include <string>
#include <vector>

BeginConstexprTest(LittleEndianInputBitIterTest, AppendBits) {
    std::vector<uint8_t> bytes = {0b101011};
    auto source = koda::MakeLittleEndianInputSource(bytes.begin());
    koda::LittleEndianInputBitIter iter{source};

    ConstexprAssertTrue(*iter);
    ConstexprAssertEqual(iter.Position(), 0);
    ConstexprAssertTrue(*++iter);
    ConstexprAssertEqual(iter.Position(), 1);
    ConstexprAssertFalse(*++iter);
    ConstexprAssertEqual(iter.Position(), 2);
    ConstexprAssertTrue(*++iter);
    ConstexprAssertEqual(iter.Position(), 3);
    ConstexprAssertFalse(*++iter);
    ConstexprAssertEqual(iter.Position(), 4);
    ConstexprAssertTrue(*++iter);
    ConstexprAssertEqual(iter.Position(), 5);
    ConstexprAssertFalse(*++iter);
    ConstexprAssertEqual(iter.Position(), 6);
    ConstexprAssertFalse(*++iter);
    ConstexprAssertEqual(iter.Position(), 7);
    ++iter;
    ConstexprAssertEqual(iter.Position(), 0);

    auto final = koda::MakeLittleEndianInputSource(bytes.begin());
    auto sentinel = koda::MakeLittleEndianInputSource(bytes.end());
    ConstexprAssertNotEqual(iter, koda::LittleEndianInputBitIter{final});
    ConstexprAssertEqual(iter, koda::LittleEndianInputBitIter{sentinel});
}
EndConstexprTest;

BeginConstexprTest(BigEndianInputBitIterTest, AppendBits) {
    std::vector<uint8_t> bytes = {0b101011};
    auto source = koda::MakeBigEndianInputSource(bytes.begin());
    koda::BigEndianInputBitIter iter{source};

    ConstexprAssertFalse(*iter);
    ConstexprAssertEqual(iter.Position(), 7);
    ConstexprAssertFalse(*++iter);
    ConstexprAssertEqual(iter.Position(), 6);
    ConstexprAssertTrue(*++iter);
    ConstexprAssertEqual(iter.Position(), 5);
    ConstexprAssertFalse(*++iter);
    ConstexprAssertEqual(iter.Position(), 4);
    ConstexprAssertTrue(*++iter);
    ConstexprAssertEqual(iter.Position(), 3);
    ConstexprAssertFalse(*++iter);
    ConstexprAssertEqual(iter.Position(), 2);
    ConstexprAssertTrue(*++iter);
    ConstexprAssertEqual(iter.Position(), 1);
    ConstexprAssertTrue(*++iter);
    ConstexprAssertEqual(iter.Position(), 0);
    ++iter;
    ConstexprAssertEqual(iter.Position(), 7);

    auto final = koda::MakeBigEndianInputSource(bytes.begin());
    auto sentinel = koda::MakeBigEndianInputSource(bytes.end());
    ConstexprAssertNotEqual(iter, koda::BigEndianInputBitIter{final});
    ConstexprAssertEqual(iter, koda::BigEndianInputBitIter{sentinel});
}
EndConstexprTest;

BeginConstexprTest(LittleEndianOutputBitIterTest, AppendBits) {
    std::vector<uint8_t> result;

    auto source =
        koda::MakeLittleEndianOutputSource(koda::BackInserterIterator{result});
    koda::LittleEndianOutputBitIter iter{source};

    ConstexprAssertEqual(iter.Position(), 0);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 1);
    *++iter = 0;
    ConstexprAssertEqual(iter.Position(), 2);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 3);
    *++iter = 0;
    ConstexprAssertEqual(iter.Position(), 4);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 5);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 6);
    *++iter = 0;
    ConstexprAssertEqual(iter.Position(), 7);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 0);

    ConstexprAssertFalse(result.empty());
    ConstexprAssertEqual(result.front(), 0b10110101);
}
EndConstexprTest;

BeginConstexprTest(BigEndianOutputBitIterIterTest, AppendBits) {
    std::vector<uint8_t> result;

    auto source =
        koda::MakeBigEndianOutputSource(koda::BackInserterIterator{result});
    koda::BigEndianOutputBitIter iter{source};

    ConstexprAssertEqual(iter.Position(), 7);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 6);
    *++iter = 0;
    ConstexprAssertEqual(iter.Position(), 5);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 4);
    *++iter = 0;
    ConstexprAssertEqual(iter.Position(), 3);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 2);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 1);
    *++iter = 0;
    ConstexprAssertEqual(iter.Position(), 0);
    *++iter = 1;
    ConstexprAssertEqual(iter.Position(), 7);

    ConstexprAssertFalse(result.empty());
    ConstexprAssertEqual(result.front(), 0b10101101);
}
EndConstexprTest;
