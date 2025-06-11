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
    { iter.Position() } -> std::same_as<size_t>;
};

template <class Iter>
concept BitOutputIterator =
    std::output_iterator<Iter, bool> && requires(Iter iter) {
        { iter.Position() } -> std::same_as<size_t>;
    };

template <class Range>
concept BitInputRange = std::ranges::input_range<Range> &&
                        BitInputIterator<std::ranges::iterator_t<Range>>;

template <class Range>
concept BitOutputRange = std::ranges::output_range<Range, bool> &&
                         BitOutputIterator<std::ranges::iterator_t<Range>>;

template <typename Iter>
class BitIteratorBase {
   public:
    [[nodiscard]] friend constexpr bool operator==(
        BitIteratorBase const& left, BitIteratorBase const& right) noexcept {
        return (left.iter_ == right.iter_) &&
               (left.bit_iter_ == right.bit_iter_);
    }

    [[nodiscard]] friend constexpr bool operator==(
        BitIteratorBase const& left, std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return left.iter_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel, BitIteratorBase const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return sentinel == right.iter_;
    }

    [[nodiscard]] static inline consteval size_t ByteLength() noexcept {
        return sizeof(TemporaryTp) * CHAR_BIT;
    }

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return bit_iter_;
    }

   protected:
    using TemporaryTp = std::iter_value_t<Iter>;

    explicit constexpr BitIteratorBase(
        Iter iterator,
        uint16_t
            start_iter) noexcept(std::is_nothrow_move_constructible_v<Iter>)
        : iter_{std::move(iterator)}, bit_iter_{start_iter} {}

    explicit constexpr BitIteratorBase(uint16_t start_iter) noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        requires std::constructible_from<Iter>
        : bit_iter_{start_iter} {}

    Iter iter_ = {};
    mutable TemporaryTp current_value_ = {};
    uint16_t bit_iter_;
    mutable bool should_fetch_ = true;
};

template <typename Iter>
class LittleEndianInputBitIter : public BitIteratorBase<Iter> {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr LittleEndianInputBitIter(Iter iterator) noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        : BitIteratorBase<Iter>{std::move(iterator), 0} {}

    explicit constexpr LittleEndianInputBitIter() noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        requires std::constructible_from<Iter>
        : BitIteratorBase<Iter>{0} {}

    [[nodiscard]] constexpr bit operator*() const noexcept {
        if (this->should_fetch_) {
            this->current_value_ = *(this->iter_);
            this->should_fetch_ = false;
        }
        return this->current_value_ & kParityBitMask;
    }

    constexpr LittleEndianInputBitIter& operator++() noexcept {
        if (++(this->bit_iter_) == this->ByteLength()) {
            this->bit_iter_ = 0;
            this->should_fetch_ = true;
        } else {
            this->current_value_ >>= 1;
        }
        return *this;
    }

    [[nodiscard]] constexpr LittleEndianInputBitIter operator++(int) noexcept {
        auto temp = *this;
        ++(*this);
        return temp;
    }

   private:
    using TemporaryTp = std::iter_value_t<Iter>;

    static constexpr TemporaryTp kParityBitMask = 1;
};

template <typename Iter>
class LittleEndianOutputBitIter : public BitIteratorBase<Iter> {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr LittleEndianOutputBitIter(Iter iterator) noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        : BitIteratorBase<Iter>{std::move(iterator), 0} {}

    explicit constexpr LittleEndianOutputBitIter() noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        requires std::constructible_from<Iter>
        : BitIteratorBase<Iter>{0} {}

    constexpr LittleEndianOutputBitIter& operator=(bit value) noexcept {
        this->current_value_ |= (value ? 1 : 0) << this->bit_iter_;
        if (++(this->bit_iter_) == this->ByteLength()) {
            *(this->iter_)++ = this->current_value_;
            this->bit_iter_ = this->current_value_ = 0;
        }
        return *this;
    }

    [[nodiscard]] constexpr LittleEndianOutputBitIter& operator*(
        void) noexcept {
        return *this;
    }

    constexpr LittleEndianOutputBitIter& operator++() noexcept { return *this; }

    [[nodiscard]] constexpr LittleEndianOutputBitIter& operator++(
        int) noexcept {
        return *this;
    }
};

template <typename Iter>
class BigEndianInputBitIter : public BitIteratorBase<Iter> {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr BigEndianInputBitIter(Iter iterator) noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        : BitIteratorBase<Iter>{std::move(iterator), this->ByteLength() - 1} {}

    explicit constexpr BigEndianInputBitIter() noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        requires std::constructible_from<Iter>
        : BitIteratorBase<Iter>{this->ByteLength() - 1} {}

    [[nodiscard]] constexpr bit operator*() const noexcept {
        if (this->should_fetch_) {
            this->current_value_ = *(this->iter_);
            this->should_fetch_ = false;
        }
        return this->current_value_ & kSignBitMask;
    }

    constexpr BigEndianInputBitIter& operator++() noexcept {
        if (!((this->bit_iter_)--)) {
            this->bit_iter_ = this->ByteLength() - 1;
            this->should_fetch_ = true;
        } else {
            this->current_value_ <<= 1;
        }
        return *this;
    }

    [[nodiscard]] constexpr BigEndianInputBitIter operator++(int) noexcept {
        auto temp = *this;
        ++(*this);
        return temp;
    }

   private:
    using TemporaryTp = std::iter_value_t<Iter>;

    static constexpr TemporaryTp kSignBitMask = 1 << (sizeof(TemporaryTp) - 1);
};

template <typename Iter>
class BigEndianOutputBitIter : public BitIteratorBase<Iter> {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr BigEndianOutputBitIter(Iter iterator) noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        : BitIteratorBase<Iter>{std::move(iterator), this->ByteLength() - 1} {}

    explicit constexpr BigEndianOutputBitIter() noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        requires std::constructible_from<Iter>
        : BitIteratorBase<Iter>{this->ByteLength() - 1} {}

    constexpr BigEndianOutputBitIter& operator=(bit value) noexcept {
        this->current_value_ = (this->current_value_ << 1) | (value ? 0 : 1);
        if (!((this->bit_iter_)--)) {
            *(this->iter_)++ = this->current_value_;
            this->current_value_ = 0;
            this->bit_iter_ = this->ByteLength() - 1;
        }
        return *this;
    }

    [[nodiscard]] constexpr BigEndianOutputBitIter& operator*() noexcept {
        return *this;
    }

    constexpr BigEndianOutputBitIter& operator++() noexcept { return *this; }

    [[nodiscard]] constexpr BigEndianOutputBitIter& operator++(int) noexcept {
        return *this;
    }
};

// template <typename Range>
// class LittleEndianInputBitRangeWrapper {
//     using BeginSource =
//         InputBitIteratorSource<std::ranges::iterator_t<const Range>>;
//     using EndSource =
//         InputBitIteratorSource<std::ranges::sentinel_t<const Range>>;

//    public:
//     using iterator_type =
//         LittleEndianInputBitIter<std::ranges::iterator_t<const Range>>;
//     using sentinel_type =
//         LittleEndianInputBitIter<std::ranges::sentinel_t<const Range>>;

//     constexpr explicit LittleEndianInputBitRangeWrapper(const Range&
//     range)
//         : begin_{BeginSource::MakeLittleEndianSource(
//               std::ranges::begin(range))},
//           end_{EndSource::MakeLittleEndianSource(std::ranges::end(range))}
//           {}

//     [[nodiscard]] constexpr iterator_type begin() {
//         return iterator_type{begin_};
//     }

//     [[nodiscard]] constexpr sentinel_type end() { return
//     sentinel_type{end_}; }

//    private:
//     BeginSource begin_;
//     EndSource end_;
// };

// template <typename Range>
//     requires std::same_as<std::ranges::sentinel_t<Range>,
//                           std::default_sentinel_t>
// class LittleEndianInputBitRangeWrapper<Range> {
//     using BeginSource =
//         InputBitIteratorSource<std::ranges::iterator_t<const Range>>;

//    public:
//     using iterator_type =
//         LittleEndianInputBitIter<std::ranges::iterator_t<const Range>>;

//     constexpr explicit LittleEndianInputBitRangeWrapper(const Range&
//     range)
//         : begin_{
//               BeginSource::MakeLittleEndianSource(std::ranges::begin(range))}
//               {}

//     [[nodiscard]] constexpr iterator_type begin() {
//         return iterator_type{begin_};
//     }

//     [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept
//     {
//         return std::default_sentinel;
//     }

//    private:
//     BeginSource begin_;
// };

// template <typename Range>
// class BigEndianInputBitRangeWrapper {
//     using BeginSource =
//         InputBitIteratorSource<std::ranges::iterator_t<const Range>>;
//     using EndSource =
//         InputBitIteratorSource<std::ranges::sentinel_t<const Range>>;

//    public:
//     using iterator_type =
//         BigEndianInputBitIter<std::ranges::iterator_t<const Range>>;
//     using sentinel_type =
//         BigEndianInputBitIter<std::ranges::sentinel_t<const Range>>;

//     constexpr explicit BigEndianInputBitRangeWrapper(const Range& range)
//         :
//         begin_{BeginSource::MakeBigEndianSource(std::ranges::begin(range))},
//           end_{EndSource::MakeBigEndianSource(std::ranges::end(range))}
//           {}

//     [[nodiscard]] constexpr iterator_type begin() {
//         return iterator_type{begin_};
//     }

//     [[nodiscard]] constexpr sentinel_type end() { return
//     sentinel_type{end_}; }

//    private:
//     BeginSource begin_;
//     EndSource end_;
// };

// template <typename Range>
//     requires std::same_as<std::ranges::sentinel_t<Range>,
//                           std::default_sentinel_t>
// class BigEndianInputBitRangeWrapper<Range> {
//     using BeginSource =
//         InputBitIteratorSource<std::ranges::iterator_t<const Range>>;

//    public:
//     using iterator_type =
//         BigEndianInputBitIter<std::ranges::iterator_t<const Range>>;

//     constexpr explicit BigEndianInputBitRangeWrapper(const Range& range)
//         :
//         begin_{BeginSource::MakeBigEndianSource(std::ranges::begin(range))}
//         {}

//     [[nodiscard]] constexpr iterator_type begin() {
//         return iterator_type{begin_};
//     }

//     [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept
//     {
//         return std::default_sentinel;
//     }

//    private:
//     BeginSource begin_;
// };

// template <typename Range>
// class LittleEndianOutputBitRangeWrapper {
//     using BeginSource =
//     OutputBitIteratorSource<std::ranges::iterator_t<Range>>; using
//     EndSource = OutputBitIteratorSource<std::ranges::sentinel_t<Range>>;

//    public:
//     using iterator_type =
//         LittleEndianOutputBitIter<std::ranges::iterator_t<Range>>;
//     using sentinel_type =
//         LittleEndianOutputBitIter<std::ranges::sentinel_t<Range>>;

//     constexpr explicit LittleEndianOutputBitRangeWrapper(Range&& range)
//         : begin_{BeginSource::MakeLittleEndianSource(
//               std::ranges::begin(range))},
//           end_{EndSource::MakeLittleEndianSource(std::ranges::end(range))}
//           {}

//     [[nodiscard]] constexpr iterator_type begin() {
//         return iterator_type{begin_};
//     }

//     [[nodiscard]] constexpr sentinel_type end() { return
//     sentinel_type{end_}; }

//    private:
//     BeginSource begin_;
//     EndSource end_;
// };

// template <typename Range>
//     requires std::same_as<std::ranges::sentinel_t<Range>,
//                           std::default_sentinel_t>
// class LittleEndianOutputBitRangeWrapper<Range> {
//     using BeginSource =
//     OutputBitIteratorSource<std::ranges::iterator_t<Range>>;

//    public:
//     using iterator_type =
//         LittleEndianOutputBitIter<std::ranges::iterator_t<Range>>;

//     constexpr explicit LittleEndianOutputBitRangeWrapper(Range&& range)
//         : begin_{
//               BeginSource::MakeLittleEndianSource(std::ranges::begin(range))}
//               {}

//     [[nodiscard]] constexpr iterator_type begin() {
//         return iterator_type{begin_};
//     }

//     [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept
//     {
//         return std::default_sentinel;
//     }

//    private:
//     BeginSource begin_;
// };

// template <typename Range>
// class BigEndianOutputBitRangeWrapper {
//     using BeginSource =
//     OutputBitIteratorSource<std::ranges::iterator_t<Range>>; using
//     EndSource = OutputBitIteratorSource<std::ranges::sentinel_t<Range>>;

//    public:
//     using iterator_type =
//         BigEndianOutputBitIter<std::ranges::iterator_t<Range>>;
//     using sentinel_type =
//         BigEndianOutputBitIter<std::ranges::sentinel_t<Range>>;

//     constexpr explicit BigEndianOutputBitRangeWrapper(Range&& range)
//         :
//         begin_{BeginSource::MakeBigEndianSource(std::ranges::begin(range))},
//           end_{EndSource::MakeBigEndianSource(std::ranges::end(range))}
//           {}

//     [[nodiscard]] constexpr iterator_type begin() {
//         return iterator_type{begin_};
//     }

//     [[nodiscard]] constexpr sentinel_type end() { return
//     sentinel_type{end_}; }

//    private:
//     BeginSource begin_;
//     EndSource end_;
// };

// template <typename Range>
//     requires std::same_as<std::ranges::sentinel_t<Range>,
//                           std::default_sentinel_t>
// class BigEndianOutputBitRangeWrapper<Range> {
//     using BeginSource =
//     OutputBitIteratorSource<std::ranges::iterator_t<Range>>;

//    public:
//     using iterator_type =
//         BigEndianOutputBitIter<std::ranges::iterator_t<Range>>;

//     constexpr explicit BigEndianOutputBitRangeWrapper(Range&& range)
//         :
//         begin_{BeginSource::MakeBigEndianSource(std::ranges::begin(range))}
//         {}

//     [[nodiscard]] constexpr iterator_type begin() {
//         return iterator_type{begin_};
//     }

//     [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept
//     {
//         return std::default_sentinel;
//     }

//    private:
//     BeginSource begin_;
// };

}  // namespace koda

#include <koda/utils/bit_iterator.tpp>
