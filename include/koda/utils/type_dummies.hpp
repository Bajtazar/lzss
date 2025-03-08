#pragma once

#include <koda/utils/bit_iterator.hpp>

#include <cinttypes>

namespace koda {

struct DummyBinaryInputIterator {
    using difference_type = std::ptrdiff_t;
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
    using difference_type = std::ptrdiff_t;
    using value_type = uint8_t;

    DummyBinaryOutputIterator() = delete;

    uint8_t& operator*() const;

    DummyBinaryOutputIterator& operator++();

    DummyBinaryOutputIterator& operator++(int);
};

using DummyLittleEndianOutputBitIter =
    LittleEndianOutputBitIter<DummyBinaryOutputIterator>;
using DummyBigEndianOutputBitIter =
    BigEndianOutputBitIter<DummyBinaryOutputIterator>;

}  // namespace koda
