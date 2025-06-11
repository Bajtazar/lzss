#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
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
EndConstexprTest;

BeginConstexprTest(LittleEndianOutputBitIterTest, AppendBits) {
    std::vector<uint8_t> result;
    koda::LittleEndianOutputBitIter iter{koda::BackInserterIterator{result}};

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
EndConstexprTest;

BeginConstexprTest(BigEndianInputIterIterTest, AppendBits) {
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
EndConstexprTest;

BeginConstexprTest(BigEndianOutputBitIterIterTest, AppendBits) {
    std::vector<uint8_t> result;
    koda::BigEndianOutputBitIter iter{koda::BackInserterIterator{result}};

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
EndConstexprTest;
