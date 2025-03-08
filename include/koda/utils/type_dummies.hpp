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
struct DummyInputRange {
    using iterator = DummyInputIterator<Tp>;

    iterator begin();

    iterator end();
};

struct DummyBitInputRange {
    using iterator = DummyBitInputIterator;

    iterator begin();

    iterator end();
};

template <typename Tp>
struct DummyOutputRange {
    using iterator = DummyOutputIterator<Tp>;

    iterator begin();

    iterator end();
};

struct DummyBitOutputRange {
    using iterator = DummyBitOutputIterator;

    iterator begin();

    iterator end();
};

struct DummyEncoder {
    void operator()(std::ranges::input_range auto, BitOutputRange auto);
};

template <typename Tp>
struct DummyDecoder {
    void operator()(BitInputRange auto, std::ranges::output_range<Tp> auto);
};

}  // namespace koda
