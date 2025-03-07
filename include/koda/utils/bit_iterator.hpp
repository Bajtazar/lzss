#pragma once

#include <cinttypes>
#include <climits>
#include <concepts>
#include <iterator>
#include <ranges>

namespace koda {

/**
 * Defines the category of the input bit iterators
 */
struct BitInputIteratorTag : public std::input_iterator_tag {};

/**
 * Defines the category of the input bit iterators
 */
struct BitOutputIteratorTag : public std::output_iterator_tag {};

/**
 * Checks whether the given iterator is the input bit iterator
 *
 * @tparam Iter the iterator type
 */
template <class Iter>
concept BitInputIterator =
    std::input_iterator<Iter> &&
    requires { typename Iter::iterator_category; } &&
    std::derived_from<typename Iter::iterator_category, BitInputIteratorTag> &&
    requires(Iter iter) {
        { iter.ReadByte() } -> std::same_as<std::byte>;
        { iter.Position() } -> std::same_as<uint8_t>;
    };

/**
 * Checks whether the given iterator is the output bit iterator
 *
 * @tparam Iter the iterator type
 */
template <class Iter>
concept BitOutputIterator =
    std::output_iterator<Iter, bool> &&
    requires { typename Iter::iterator_category; } &&
    std::derived_from<typename Iter::iterator_category, BitOutputIteratorTag> &&
    requires(Iter iter, std::byte byte) {
        { iter.SaveByte(byte) } -> std::same_as<void>;
        { iter.Position() } -> std::same_as<uint8_t>;
    };

/**
 * Checks whether the given range uses a bit input iterator
 *
 * @tparam Range the checked range
 */
template <class Range>
concept BitInputRange = std::ranges::input_range<Range> &&
                        BitInputIterator<std::ranges::iterator_t<Range>>;

/**
 * Checks whether the given range uses a bit output iterator
 *
 * @tparam Range the checked range
 */
template <class Range>
concept BitOutputRange = std::ranges::output_range<Range, bool> &&
                         BitOutputIterator<std::ranges::iterator_t<Range>>;

/**
 * Checks whether two types takes the same ammount of space
 * in the memory
 *
 * @tparam Tp the first type
 * @tparam Up the second type
 */
template <typename Tp, typename Up>
concept SameSize = (sizeof(Tp) == sizeof(Up));

/**
 * Checks whether the given iterator accepts a byte-like type
 *
 * @tparam Iter the checked iterator
 */
template <class Iter>
concept ByteInputIterator = std::input_iterator<Iter> && requires(Iter iter) {
    { *iter } -> SameSize<std::byte>;
};

/**
 * Checks whether the given iterator returns a byte-like type
 *
 * @tparam Iter the checked iterator
 */
template <class Iter>
concept ByteOutputIterator = std::output_iterator<Iter, uint8_t> &&
                             requires(Iter iter, uint8_t c) { *iter = c; };

/**
 * Iterator returning the values of the individual bits in the
 * little endian manner
 *
 * @tparam Iter the iterator type that iterates through
 * an individual bytes
 */
template <ByteInputIterator Iter>
class LittleEndianInputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;
    using iterator_category = BitInputIteratorTag;

    /**
     * Constructs a new little endian bit input iterator from
     * the given iterator
     *
     * @param iter the constant reference to the iterator
     */
    explicit constexpr LittleEndianInputBitIter(Iter const& iter) noexcept
        : iter_{iter}, bit_iter_{0} {}

    /**
     * Constructs a new little endian bit input iterator from
     * the given iterator
     *
     * @param iter the rvalue reference to the iterator
     */
    explicit constexpr LittleEndianInputBitIter(Iter&& iter) noexcept
        : iter_{std::move(iter)}, bit_iter_{0} {}

    /**
     * Constructs a new little endian bit input iterator
     */
    explicit constexpr LittleEndianInputBitIter(void) noexcept = default;

    /**
     * Returns a value of the currently examined bit
     *
     * @return the value of the currently examined bit
     */
    [[nodiscard]] constexpr bit operator*(void) const noexcept {
        return ((1 << bit_iter_) & (*iter_)) >> bit_iter_;
    }

    /**
     * Increments iterator by one
     *
     * @return reference to this object
     */
    constexpr LittleEndianInputBitIter& operator++(void) noexcept;

    /**
     * Post-increments iterator by one and returns copy
     * of the object
     *
     * @return the copied object
     */
    [[nodiscard]] constexpr LittleEndianInputBitIter operator++(int) noexcept {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    /**
     * Checks whether two iterators are equal
     *
     * @param left the left iterator
     * @param right the right iterator
     * @return whether two iterators are equal
     */
    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianInputBitIter const& left,
        LittleEndianInputBitIter const& right) noexcept {
        return left.iter_ == right.iter_;
    }

    /**
     * Returns a number of bits in byte
     *
     * @return the number of bits in byte
     */
    [[nodiscard]] static inline consteval uint8_t ByteLength(void) noexcept {
        return CHAR_BIT;
    }

    /**
     * Jums to the begining of the next byte
     */
    constexpr void SkipToNextByte(void) noexcept {
        ++iter_;
        bit_iter_ = 0;
    }

    /**
     * Reads the entire byte
     *
     * @return the readed byte
     */
    [[nodiscard]] constexpr std::byte ReadByte(void) noexcept;

    /**
     * Returns the current bit Position
     *
     * @return the current bit Position
     */
    [[nodiscard]] constexpr uint8_t Position(void) const noexcept {
        return bit_iter_;
    }

   private:
    Iter iter_;
    uint8_t bit_iter_;
};

/**
 * Iterator saving the values of the individual bits in the
 * little endian manner
 *
 * @tparam Iter the iterator type that iterates through
 * an individual bytes
 */
template <ByteOutputIterator Iter>
class LittleEndianOutputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;
    using iterator_category = BitOutputIteratorTag;

    /**
     * Constructs a new little endian bit output iterator from
     * the given iterator
     *
     * @param iter the constant reference to the iterator
     */
    explicit constexpr LittleEndianOutputBitIter(Iter const& iter) noexcept
        : iter_{iter}, temporary_{0}, bit_iter_{0} {}

    /**
     * Constructs a new little endian bit output iterator from
     * the given iterator
     *
     * @param iter the rvalue reference to the iterator
     */
    explicit constexpr LittleEndianOutputBitIter(Iter&& iter) noexcept
        : iter_{std::move(iter)}, temporary_{0}, bit_iter_{0} {}

    /**
     * Constructs a new little endian bit output iterator
     */
    explicit constexpr LittleEndianOutputBitIter(void) noexcept = default;

    /**
     * Saves the given value in the wrapped output iterator
     *
     * @param value the value of the current bit
     * @return reference to this object
     */
    constexpr LittleEndianOutputBitIter& operator=(bit value) noexcept;

    /**
     * Returns the reference to this object [allows to assing
     * the value]
     *
     * @return the reference to this object
     */
    [[nodiscard]] constexpr LittleEndianOutputBitIter& operator*(
        void) noexcept {
        return *this;
    }

    /**
     * Returns the reference to this object
     *
     * @return reference to this object
     */
    constexpr LittleEndianOutputBitIter& operator++(void) noexcept {
        return *this;
    }

    /**
     * Returns the copy of this object
     *
     * @return the copied object
     */
    [[nodiscard]] constexpr LittleEndianOutputBitIter& operator++(
        int) noexcept {
        return *this;
    }

    /**
     * Checks whether two iterators are equal
     *
     * @param left the left iterator
     * @param right the right iterator
     * @return whether two iterators are equal
     */
    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianOutputBitIter const& left,
        LittleEndianOutputBitIter const& right) noexcept {
        return left.iter_ == right.iter_;
    }

    /**
     * Returns a number of bits in byte
     *
     * @return the number of bits in byte
     */
    [[nodiscard]] static inline consteval uint8_t ByteLength(void) noexcept {
        return CHAR_BIT;
    }

    /**
     * Jums to the begining of the next byte
     */
    constexpr void SkipToNextByte(void) noexcept;

    /**
     * Saves the entire byte
     *
     * @param byte the saved byte
     */
    constexpr void SaveByte(uint8_t byte) noexcept;

    /**
     * Returns the current bit Position
     *
     * @return the current bit Position
     */
    [[nodiscard]] constexpr uint8_t Position(void) const noexcept {
        return bit_iter_;
    }

   private:
    Iter iter_;
    uint8_t temporary_;
    uint8_t bit_iter_;
};

/**
 * Iterator returning the values of the individual bits in the
 * big endian manner
 *
 * @tparam Iter the iterator type that iterates through
 * an individual bytes
 */
template <ByteInputIterator Iter>
class BigEndianInputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;
    using iterator_category = BitInputIteratorTag;

    /**
     * Constructs a new big endian bit input iterator from
     * the given iterator
     *
     * @param iter the constant reference to the iterator
     */
    explicit constexpr BigEndianInputBitIter(Iter const& iter) noexcept
        : iter_{iter}, bit_iter_{7} {}

    /**
     * Constructs a new big endian bit input iterator from
     * the given iterator
     *
     * @param iter the rvalue reference to the iterator
     */
    explicit constexpr BigEndianInputBitIter(Iter&& iter) noexcept
        : iter_{std::move(iter)}, bit_iter_{7} {}

    /**
     * Constructs a new big endian bit input iterator
     */
    explicit constexpr BigEndianInputBitIter(void) noexcept = default;

    /**
     * Returns a value of the currently examined bit
     *
     * @return the value of the currently examined bit
     */
    [[nodiscard]] constexpr bit operator*(void) const noexcept {
        return ((1 << bit_iter_) & (*iter_)) >> bit_iter_;
    }

    /**
     * Increments iterator by one
     *
     * @return reference to this object
     */
    constexpr BigEndianInputBitIter& operator++(void) noexcept;

    /**
     * Post-increments iterator by one and returns copy
     * of the object
     *
     * @return the copied object
     */
    [[nodiscard]] constexpr BigEndianInputBitIter operator++(int) noexcept {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    /**
     * Checks whether two iterators are equal
     *
     * @param left the left iterator
     * @param right the right iterator
     * @return whether two iterators are equal
     */
    [[nodiscard]] friend constexpr bool operator==(
        BigEndianInputBitIter const& left,
        BigEndianInputBitIter const& right) noexcept {
        return left.iter_ == right.iter_;
    }

    /**
     * Returns a number of bits in byte
     *
     * @return the number of bits in byte
     */
    [[nodiscard]] static inline consteval uint8_t ByteLength(void) noexcept {
        return CHAR_BIT;
    }

    /**
     * Jums to the begining of the next byte
     */
    constexpr void SkipToNextByte(void) noexcept {
        ++iter_;
        bit_iter_ = 7;
    }

    /**
     * Reads the entire byte
     *
     * @return the readed byte
     */
    [[nodiscard]] constexpr std::byte ReadByte(void) noexcept;

    /**
     * Returns the current bit Position
     *
     * @return the current bit Position
     */
    [[nodiscard]] constexpr uint8_t Position(void) const noexcept {
        return bit_iter_;
    }

   private:
    Iter iter_;
    uint8_t bit_iter_;
};

/**
 * Iterator saving the values of the individual bits in the
 * big endian manner
 *
 * @tparam Iter the iterator type that iterates through
 * an individual bytes
 */
template <ByteOutputIterator Iter>
class BigEndianOutputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;
    using iterator_category = BitOutputIteratorTag;

    /**
     * Constructs a new big endian bit output iterator from
     * the given iterator
     *
     * @param iter the constant reference to the iterator
     */
    explicit constexpr BigEndianOutputBitIter(Iter const& iter) noexcept
        : iter_{iter}, temporary_{0}, bit_iter_{7} {}

    /**
     * Constructs a new big endian bit output iterator from
     * the given iterator
     *
     * @param iter the rvalue reference to the iterator
     */
    explicit constexpr BigEndianOutputBitIter(Iter&& iter) noexcept
        : iter_{std::move(iter)}, temporary_{0}, bit_iter_{7} {}

    /**
     * Constructs a new big endian bit output iterator
     */
    explicit constexpr BigEndianOutputBitIter(void) noexcept = default;

    /**
     * Saves the given value in the wrapped output iterator
     *
     * @param value the value of the current bit
     * @return reference to this object
     */
    constexpr BigEndianOutputBitIter& operator=(bit value) noexcept;

    /**
     * Returns the reference to this object [allows to assing
     * the value]
     *
     * @return the reference to this object
     */
    [[nodiscard]] constexpr BigEndianOutputBitIter& operator*(void) noexcept {
        return *this;
    }

    /**
     * Returns the reference to this object
     *
     * @return reference to this object
     */
    constexpr BigEndianOutputBitIter& operator++(void) noexcept {
        return *this;
    }

    /**
     * Returns the copy of this object
     *
     * @return the copied object
     */
    [[nodiscard]] constexpr BigEndianOutputBitIter& operator++(int) noexcept {
        return *this;
    }

    /**
     * Checks whether two iterators are equal
     *
     * @param left the left iterator
     * @param right the right iterator
     * @return whether two iterators are equal
     */
    [[nodiscard]] friend constexpr bool operator==(
        BigEndianOutputBitIter const& left,
        BigEndianOutputBitIter const& right) noexcept {
        return left.iter_ == right.iter_;
    }

    /**
     * Returns a number of bits in byte
     *
     * @return the number of bits in byte
     */
    [[nodiscard]] static inline consteval uint8_t ByteLength(void) noexcept {
        return CHAR_BIT;
    }

    /**
     * Jums to the begining of the next byte
     */
    constexpr void SkipToNextByte(void) noexcept;

    /**
     * Saves the entire byte
     *
     * @param byte the saved byte
     */
    constexpr void SaveByte(uint8_t byte) noexcept;

    /**
     * Returns the current bit Position
     *
     * @return the current bit Position
     */
    [[nodiscard]] constexpr uint8_t Position(void) const noexcept {
        return bit_iter_;
    }

   private:
    Iter iter_;
    uint8_t temporary_;
    uint8_t bit_iter_;
};

}  // namespace koda

#include <koda/utils/bit_iterator.tpp>
