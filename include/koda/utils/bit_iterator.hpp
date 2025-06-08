#pragma once

#include <cinttypes>
#include <climits>
#include <concepts>
#include <iterator>
#include <ranges>

namespace koda {

template <typename Tp, typename Up>
concept WeaklyEqualityComparable = requires(Tp& tp, Up& up) {
    { tp == up } -> std::same_as<bool>;
};

template <class Iter>
concept BitInputIterator = std::input_iterator<Iter> && requires(Iter iter) {
    { iter.ReadByte() } -> std::same_as<std::byte>;
    { iter.Position() } -> std::same_as<uint8_t>;
};

template <class Iter>
concept BitOutputIterator =
    std::output_iterator<Iter, uint8_t> && requires(Iter iter, uint8_t byte) {
        { iter.SaveByte(byte) } -> std::same_as<void>;
        { iter.Position() } -> std::same_as<uint8_t>;
    };

template <class Range>
concept BitInputRange = std::ranges::input_range<Range> &&
                        BitInputIterator<std::ranges::iterator_t<Range>>;

template <class Range>
concept BitOutputRange = std::ranges::output_range<Range, uint8_t> &&
                         BitOutputIterator<std::ranges::iterator_t<Range>>;

template <typename Tp, typename Up>
concept SameSize = (sizeof(Tp) == sizeof(Up));

template <class Iter>
concept ByteInputIterator = std::input_iterator<Iter> && requires(Iter iter) {
    { *iter } -> SameSize<std::byte>;
};

template <class Iter>
concept ByteOutputIterator = std::output_iterator<Iter, uint8_t> &&
                             requires(Iter iter, uint8_t c) { *iter = c; };

template <ByteInputIterator Iter>
class InputBitIteratorSource {
   public:
    using bit = bool;

    constexpr InputBitIteratorSource(
        const InputBitIteratorSource&
            other) noexcept(std::is_nothrow_copy_constructible_v<Iter>) =
        default;
    constexpr InputBitIteratorSource(InputBitIteratorSource&& other) noexcept(
        std::is_nothrow_move_constructible_v<Iter>) = default;

    constexpr InputBitIteratorSource&
    operator=(const InputBitIteratorSource& other) noexcept(
        std::is_nothrow_copy_assignable_v<Iter>) = default;
    constexpr InputBitIteratorSource&
    operator=(InputBitIteratorSource&& other) noexcept(
        std::is_nothrow_move_assignable_v<Iter>) = default;

    [[nodiscard]] static inline consteval uint8_t ByteLength() noexcept {
        return CHAR_BIT;
    }

    [[nodiscard]] constexpr bit value() const noexcept;

    constexpr void IncrementLittleEndianess() noexcept;

    constexpr void IncrementBigEndianess() noexcept;

    constexpr void SkipToNextByteLittleEndianess() noexcept;

    constexpr void SkipToNextByteBigEndianess() noexcept;

    [[nodiscard]] constexpr std::byte ReadByteLittleEndianess() noexcept;

    [[nodiscard]] constexpr std::byte ReadByteBigEndianess() noexcept;

    [[nodiscard]] constexpr uint8_t Position() const noexcept {
        return bit_iter_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        InputBitIteratorSource const& left,
        InputBitIteratorSource const& right) noexcept {
        return (left.iter_ == right.iter_) &&
               (left.bit_iter_ == right.bit_iter_);
    }

    [[nodiscard]] friend constexpr bool operator==(
        InputBitIteratorSource const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return left.iter_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        InputBitIteratorSource const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return sentinel == right.iter_;
    }

    [[nodiscard]] friend constexpr InputBitIteratorSource
    MakeLittleEndianInputBitIteratorSource(Iter iter) noexcept(
        std::is_nothrow_move_constructible_v<Iter>) {
        return InputBitIteratorSource{std::move(iter), /*bit_iter*/ 0};
    }

    [[nodiscard]] friend constexpr InputBitIteratorSource
    MakeBigEndianInputBitIteratorSource(Iter iter) noexcept(
        std::is_nothrow_move_constructible_v<Iter>) {
        return InputBitIteratorSource{std::move(iter),
                                      /*bit_iter*/ ByteLength() - 1};
    }

    constexpr ~InputBitIteratorSource() noexcept(
        std::is_nothrow_destructible_v<Iter>) = default;

   private:
    constexpr explicit InputBitIteratorSource(
        Iter iter,
        uint8_t bit_iter) noexcept(std::is_nothrow_move_constructible_v<Iter>)
        : iter_{std::move(iter)}, bit_iter_{bit_iter} {}

    Iter iter_;
    uint8_t bit_iter_;
};

template <ByteInputIterator Iter>
class LittleEndianInputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr LittleEndianInputBitIter(Iter const& iter) noexcept
        : iter_{iter}, bit_iter_{0} {}

    explicit constexpr LittleEndianInputBitIter(Iter&& iter) noexcept
        : iter_{std::move(iter)}, bit_iter_{0} {}

    explicit constexpr LittleEndianInputBitIter() noexcept = default;

    [[nodiscard]] constexpr bit operator*() const noexcept {
        return ((1 << bit_iter_) & (*iter_)) >> bit_iter_;
    }

    constexpr LittleEndianInputBitIter& operator++() noexcept;

    [[nodiscard]] constexpr LittleEndianInputBitIter operator++(int) noexcept {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianInputBitIter const& left,
        LittleEndianInputBitIter const& right) noexcept {
        return left.iter_ == right.iter_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianInputBitIter const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return left.iter_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        LittleEndianInputBitIter const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return sentinel == right.iter_;
    }

    [[nodiscard]] static inline consteval uint8_t ByteLength() noexcept {
        return CHAR_BIT;
    }

    constexpr void SkipToNextByte() noexcept {}

    [[nodiscard]] constexpr std::byte ReadByte() noexcept;

    [[nodiscard]] constexpr uint8_t Position() const noexcept {
        return bit_iter_;
    }

   private:
    Iter iter_;
    uint8_t bit_iter_;
};

template <ByteOutputIterator Iter>
class LittleEndianOutputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr LittleEndianOutputBitIter(Iter const& iter) noexcept
        : iter_{iter}, temporary_{0}, bit_iter_{0} {}

    explicit constexpr LittleEndianOutputBitIter(Iter&& iter) noexcept
        : iter_{std::move(iter)}, temporary_{0}, bit_iter_{0} {}

    explicit constexpr LittleEndianOutputBitIter() noexcept = default;

    constexpr LittleEndianOutputBitIter& operator=(bit value) noexcept;

    [[nodiscard]] constexpr LittleEndianOutputBitIter& operator*(
        void) noexcept {
        return *this;
    }

    constexpr LittleEndianOutputBitIter& operator++() noexcept { return *this; }

    [[nodiscard]] constexpr LittleEndianOutputBitIter& operator++(
        int) noexcept {
        return *this;
    }

    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianOutputBitIter const& left,
        LittleEndianOutputBitIter const& right) noexcept {
        return left.iter_ == right.iter_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianOutputBitIter const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return left.iter_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        LittleEndianOutputBitIter const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return sentinel == right.iter_;
    }

    [[nodiscard]] static inline consteval uint8_t ByteLength() noexcept {
        return CHAR_BIT;
    }

    constexpr void SkipToNextByte() noexcept;

    constexpr void SaveByte(uint8_t byte) noexcept;

    [[nodiscard]] constexpr uint8_t Position() const noexcept {
        return bit_iter_;
    }

   private:
    Iter iter_;
    uint8_t temporary_;
    uint8_t bit_iter_;
};

template <ByteInputIterator Iter>
class BigEndianInputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr BigEndianInputBitIter(Iter const& iter) noexcept
        : iter_{iter}, bit_iter_{7} {}

    explicit constexpr BigEndianInputBitIter(Iter&& iter) noexcept
        : iter_{std::move(iter)}, bit_iter_{7} {}

    explicit constexpr BigEndianInputBitIter() noexcept = default;

    [[nodiscard]] constexpr bit operator*() const noexcept {
        return ((1 << bit_iter_) & (*iter_)) >> bit_iter_;
    }

    constexpr BigEndianInputBitIter& operator++() noexcept;

    [[nodiscard]] constexpr BigEndianInputBitIter operator++(int) noexcept {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    [[nodiscard]] friend constexpr bool operator==(
        BigEndianInputBitIter const& left,
        BigEndianInputBitIter const& right) noexcept {
        return left.iter_ == right.iter_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        BigEndianInputBitIter const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return left.iter_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        BigEndianInputBitIter const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return sentinel == right.iter_;
    }

    [[nodiscard]] static inline consteval uint8_t ByteLength() noexcept {
        return CHAR_BIT;
    }

    constexpr void SkipToNextByte() noexcept {}

    [[nodiscard]] constexpr std::byte ReadByte() noexcept;

    [[nodiscard]] constexpr uint8_t Position() const noexcept {
        return bit_iter_;
    }

   private:
    Iter iter_;
    uint8_t bit_iter_;
};

template <ByteOutputIterator Iter>
class BigEndianOutputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr BigEndianOutputBitIter(Iter const& iter) noexcept
        : iter_{iter}, temporary_{0}, bit_iter_{7} {}

    explicit constexpr BigEndianOutputBitIter(Iter&& iter) noexcept
        : iter_{std::move(iter)}, temporary_{0}, bit_iter_{7} {}

    explicit constexpr BigEndianOutputBitIter() noexcept = default;

    constexpr BigEndianOutputBitIter& operator=(bit value) noexcept;

    [[nodiscard]] constexpr BigEndianOutputBitIter& operator*() noexcept {
        return *this;
    }

    constexpr BigEndianOutputBitIter& operator++() noexcept { return *this; }

    [[nodiscard]] constexpr BigEndianOutputBitIter& operator++(int) noexcept {
        return *this;
    }

    [[nodiscard]] friend constexpr bool operator==(
        BigEndianOutputBitIter const& left,
        BigEndianOutputBitIter const& right) noexcept {
        return left.iter_ == right.iter_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        BigEndianOutputBitIter const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return left.iter_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        BigEndianOutputBitIter const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return sentinel == right.iter_;
    }

    [[nodiscard]] static inline consteval uint8_t ByteLength() noexcept {
        return CHAR_BIT;
    }

    constexpr void SkipToNextByte() noexcept;

    constexpr void SaveByte(uint8_t byte) noexcept;

    [[nodiscard]] constexpr uint8_t Position() const noexcept {
        return bit_iter_;
    }

   private:
    Iter iter_;
    uint8_t temporary_;
    uint8_t bit_iter_;
};

}  // namespace koda

#include <koda/utils/bit_iterator.tpp>
