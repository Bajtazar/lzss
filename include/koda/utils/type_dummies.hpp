#pragma once

#include <koda/ranges/bit_iterator.hpp>

#include <cinttypes>

namespace koda {

template <typename Tp>
struct DummyInputIterator {
    using difference_type = std::ptrdiff_t;
    using value_type = Tp;

    DummyInputIterator();

    value_type operator*() const;

    DummyInputIterator& operator++();

    DummyInputIterator operator++(int);

    bool operator==(const DummyInputIterator& right) const;
};

struct DummyBitInputIterator {
    using difference_type = std::ptrdiff_t;
    using value_type = bool;

    DummyBitInputIterator();

    value_type operator*() const;

    DummyBitInputIterator& operator++();

    size_t Position() const;

    void Flush();

    DummyBitInputIterator operator++(int);

    bool operator==(const DummyBitInputIterator& right) const;
};

static_assert(BitInputIterator<DummyBitInputIterator>);

template <typename Tp>
struct DummyOutputIterator {
    using difference_type = std::ptrdiff_t;
    using value_type = Tp;

    DummyOutputIterator();

    value_type& operator*() const;

    DummyOutputIterator& operator++();

    DummyOutputIterator& operator++(int);

    bool operator==(const DummyOutputIterator& right) const;
};

struct DummyBitOutputIterator {
    using difference_type = std::ptrdiff_t;
    using value_type = bool;

    DummyBitOutputIterator();

    value_type& operator*() const;

    size_t Position() const;

    void Flush();

    DummyBitOutputIterator& operator++();

    DummyBitOutputIterator& operator++(int);

    bool operator==(const DummyBitOutputIterator& right) const;
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
