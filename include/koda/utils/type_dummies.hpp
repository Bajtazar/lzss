#pragma once

#include <koda/utils/bit_iterator.hpp>

#include <cinttypes>

namespace koda {

template <typename Tp>
struct DummyInputIterator {
    using difference_type = std::ptrdiff_t;
    using value_type = Tp;

    DummyInputIterator() = delete;

    value_type operator*() const;

    DummyInputIterator& operator++();

    void operator++(int);
};

struct DummyBitInputIterator {
    using difference_type = std::ptrdiff_t;
    using value_type = uint8_t;

    DummyBitInputIterator() = delete;

    value_type operator*() const;

    DummyBitInputIterator& operator++();

    std::byte ReadByte();

    uint8_t Position();

    void operator++(int);
};

static_assert(BitInputIterator<DummyBitInputIterator>);

template <typename Tp>
struct DummyOutputIterator {
    using difference_type = std::ptrdiff_t;
    using value_type = Tp;

    DummyOutputIterator() = delete;

    value_type& operator*() const;

    DummyOutputIterator& operator++();

    DummyOutputIterator& operator++(int);
};

struct DummyBitOutputIterator {
    using difference_type = std::ptrdiff_t;
    using value_type = uint8_t;

    DummyBitOutputIterator() = delete;

    value_type& operator*() const;

    void SaveByte(uint8_t);

    uint8_t Position();

    DummyBitOutputIterator& operator++();

    DummyBitOutputIterator& operator++(int);
};

static_assert(BitOutputIterator<DummyBitOutputIterator>);

template <typename Tp>
class DummyInputRange {
   public:
    using iterator = DummyInputIterator<Tp>;

    iterator begin();

    iterator end();
};

class DummyBitInputRange {
   public:
    using iterator = DummyBitInputIterator;

    iterator begin();

    iterator end();
};

template <typename Tp>
class DummyOutputRange {
   public:
    using iterator = DummyOutputIterator<Tp>;

    iterator begin();

    iterator end();
};

class DummyBitOutputRange {
   public:
    using iterator = DummyBitOutputIterator;

    iterator begin();

    iterator end();
};

}  // namespace koda
