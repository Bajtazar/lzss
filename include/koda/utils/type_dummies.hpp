#pragma once

#include <koda/utils/bit_iterator.hpp>

#include <cinttypes>

namespace koda {

struct DummyBinaryInputIterator {
    using value_type = uint8_t;

    DummyBinaryInputIterator() = delete;

    uint8_t operator*() const;

    DummyBinaryInputIterator& operator++();

    void operator++(int);
};

using DummyLittleEndianInputBitIter =
    LittleEndianInputBitIter<DummyBinaryInputIterator>;
using DummyBigEndianInputBitIter =
    BigEndianInputBitIter<DummyBinaryInputIterator>;

struct DummyBinaryOutputIterator {
    using value_type = uint8_t;

    DummyBinaryOutputIterator() = delete;

    uint8_t& operator*() const;

    DummyBinaryOutputIterator& operator++();

    DummyBinaryOutputIterator& operator++(int);
};

using DummyLittleEndianOutputBitIter =
    LittleEndianOutputBitIter<DummyBinaryOutputIterator>;
using DummyBigEndianOutputBitIter =
    BigEndianOutputBitIter<DummyBigEndianOutputBitIter>;

}  // namespace koda
