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
    explicit constexpr BitIteratorBase(Iter iterator) noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        : iter_{std::move(iterator)} {}

    explicit constexpr BitIteratorBase() noexcept(
        std::is_nothrow_move_constructible_v<Iter>)
        requires std::constructible_from<Iter>
    = default;

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

   protected:
    using TemporaryTp = std::iter_value_t<Iter>;

    constexpr BitIteratorBase(BitIteratorBase&& other) noexcept(
        std::is_nothrow_move_constructible_v<Iter>) = default;

    constexpr BitIteratorBase(const BitIteratorBase& other) noexcept(
        std::is_nothrow_copy_constructible_v<Iter>) = default;

    constexpr BitIteratorBase& operator=(BitIteratorBase&& other) noexcept(
        std::is_nothrow_move_assignable_v<Iter>) = default;

    constexpr BitIteratorBase& operator=(const BitIteratorBase& other) noexcept(
        std::is_nothrow_copy_assignable_v<Iter>) = default;

    Iter iter_ = {};
    mutable TemporaryTp current_value_ = {};
    uint16_t bit_iter_ = 0;
};

template <typename Iter>
class LittleEndianInputBitIter : public BitIteratorBase<Iter> {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    using BitIteratorBase<Iter>::BitIteratorBase;

    constexpr LittleEndianInputBitIter(
        LittleEndianInputBitIter&&
            other) noexcept(std::is_nothrow_move_constructible_v<Iter>) =
        default;

    constexpr LittleEndianInputBitIter(
        const LittleEndianInputBitIter&
            other) noexcept(std::is_nothrow_copy_constructible_v<Iter>) =
        default;

    constexpr LittleEndianInputBitIter&
    operator=(LittleEndianInputBitIter&& other) noexcept(
        std::is_nothrow_move_assignable_v<Iter>) = default;

    constexpr LittleEndianInputBitIter&
    operator=(const LittleEndianInputBitIter& other) noexcept(
        std::is_nothrow_copy_assignable_v<Iter>) = default;

    [[nodiscard]] constexpr bit operator*() const noexcept {
        if (should_fetch_) {
            this->current_value_ = *this->iter_ >> this->bit_iter_;
            should_fetch_ = false;
        }
        return this->current_value_ & kParityBitMask;
    }

    constexpr LittleEndianInputBitIter& operator++() noexcept {
        if (++this->bit_iter_ == this->ByteLength()) {
            this->bit_iter_ = 0;
            should_fetch_ = true;
            ++this->iter_;
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

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return this->bit_iter_;
    }

   private:
    using TemporaryTp = std::iter_value_t<Iter>;

    static constexpr TemporaryTp kParityBitMask = 1;

    mutable bool should_fetch_ = true;
};

template <typename Iter>
class LittleEndianOutputBitIter : public BitIteratorBase<Iter> {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    using BitIteratorBase<Iter>::BitIteratorBase;

    constexpr LittleEndianOutputBitIter(
        LittleEndianOutputBitIter&&
            other) noexcept(std::is_nothrow_move_constructible_v<Iter>) =
        default;

    constexpr LittleEndianOutputBitIter(
        const LittleEndianOutputBitIter&
            other) noexcept(std::is_nothrow_copy_constructible_v<Iter>) =
        default;

    constexpr LittleEndianOutputBitIter&
    operator=(LittleEndianOutputBitIter&& other) noexcept(
        std::is_nothrow_move_assignable_v<Iter>) = default;

    constexpr LittleEndianOutputBitIter&
    operator=(const LittleEndianOutputBitIter& other) noexcept(
        std::is_nothrow_copy_assignable_v<Iter>) = default;

    constexpr LittleEndianOutputBitIter& operator=(bit value) noexcept {
        this->current_value_ |= (value ? 1 : 0) << this->bit_iter_;
        if (++this->bit_iter_ == this->ByteLength()) {
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

    constexpr void Flush() noexcept {
        if (this->bit_iter_) {
            *(this->iter_)++ = this->current_value_;
            this->bit_iter_ = this->current_value_ = 0;
        }
    }

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return this->bit_iter_;
    }
};

template <typename Iter>
class BigEndianInputBitIter : public BitIteratorBase<Iter> {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    using BitIteratorBase<Iter>::BitIteratorBase;

    constexpr BigEndianInputBitIter(BigEndianInputBitIter&& other) noexcept(
        std::is_nothrow_move_constructible_v<Iter>) = default;

    constexpr BigEndianInputBitIter(
        const BigEndianInputBitIter&
            other) noexcept(std::is_nothrow_copy_constructible_v<Iter>) =
        default;

    constexpr BigEndianInputBitIter&
    operator=(BigEndianInputBitIter&& other) noexcept(
        std::is_nothrow_move_assignable_v<Iter>) = default;

    constexpr BigEndianInputBitIter&
    operator=(const BigEndianInputBitIter& other) noexcept(
        std::is_nothrow_copy_assignable_v<Iter>) = default;

    [[nodiscard]] constexpr bit operator*() const noexcept {
        if (should_fetch_) {
            this->current_value_ = *this->iter_ << this->bit_iter_;
            should_fetch_ = false;
        }
        return this->current_value_ & kSignBitMask;
    }

    constexpr BigEndianInputBitIter& operator++() noexcept {
        if (++this->bit_iter_ == this->ByteLength()) {
            this->bit_iter_ = 0;
            should_fetch_ = true;
            ++this->iter_;
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

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return this->ByteLength() - 1 - this->bit_iter_;
    }

   private:
    using TemporaryTp = std::iter_value_t<Iter>;

    static constexpr TemporaryTp kSignBitMask =
        1 << (BitIteratorBase<Iter>::ByteLength() - 1);

    mutable bool should_fetch_ = true;
};

template <typename Iter>
class BigEndianOutputBitIter : public BitIteratorBase<Iter> {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    using BitIteratorBase<Iter>::BitIteratorBase;

    constexpr BigEndianOutputBitIter(BigEndianOutputBitIter&& other) noexcept(
        std::is_nothrow_move_constructible_v<Iter>) = default;

    constexpr BigEndianOutputBitIter(
        const BigEndianOutputBitIter&
            other) noexcept(std::is_nothrow_copy_constructible_v<Iter>) =
        default;

    constexpr BigEndianOutputBitIter&
    operator=(BigEndianOutputBitIter&& other) noexcept(
        std::is_nothrow_move_assignable_v<Iter>) = default;

    constexpr BigEndianOutputBitIter&
    operator=(const BigEndianOutputBitIter& other) noexcept(
        std::is_nothrow_copy_assignable_v<Iter>) = default;

    constexpr BigEndianOutputBitIter& operator=(bit value) noexcept {
        this->current_value_ = (this->current_value_ << 1) | (value ? 1 : 0);
        if (++this->bit_iter_ == this->ByteLength()) {
            *(this->iter_)++ = this->current_value_;
            this->bit_iter_ = this->current_value_ = 0;
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

    constexpr void Flush() noexcept {
        if (this->bit_iter_) {
            *(this->iter_)++ = this->current_value_
                               << (this->ByteLength() - 1 - this->bit_iter_);
            this->bit_iter_ = this->current_value_ = 0;
        }
    }

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return this->ByteLength() - 1 - this->bit_iter_;
    }
};

namespace details {

template <typename RangeTp, template <typename> class BitIteratorTp>
class BitView
    : public std::ranges::view_interface<BitView<RangeTp, BitIteratorTp>> {
   public:
    using iterator_type = std::ranges::iterator_t<RangeTp>;
    using sentinel_type = std::ranges::sentinel_t<RangeTp>;

    template <std::ranges::viewable_range RangeFwdTp>
    constexpr BitView(RangeFwdTp&& range)
        : range_{std::forward<RangeFwdTp>(range)} {}

    [[nodiscard]] constexpr BitIteratorTp<iterator_type> begin() const {
        return BitIteratorTp<iterator_type>{std::ranges::begin(range_)};
    }

    [[nodiscard]] constexpr BitIteratorTp<sentinel_type> end() const
        requires(!std::same_as<sentinel_type, std::default_sentinel_t>)
    {
        return BitIteratorTp<sentinel_type>{std::ranges::end(range_)};
    }

    [[nodiscard]] constexpr std::default_sentinel_t end() const
        requires std::same_as<sentinel_type, std::default_sentinel_t>
    {
        return std::default_sentinel;
    }

   private:
    RangeTp range_;
};

template <std::ranges::viewable_range Range,
          template <typename> class BitIteratorTp>
BitView(Range&& range)
    -> BitView<std::ranges::views::all_t<Range>, BitIteratorTp>;

template <template <typename> class BitViewTp>
struct BitViewAdaptorClosure : public std::ranges::range_adaptor_closure<
                                   BitViewAdaptorClosure<BitViewTp>> {
    template <std::ranges::viewable_range Range>
    [[nodiscard]] constexpr auto operator()(Range&& range) const {
        return BitViewTp{std::forward<Range>(range)};
    }
};

}  // namespace details

namespace ranges {

template <typename RangeTp>
using LittleEndianInputView =
    details::BitView<RangeTp, LittleEndianInputBitIter>;

template <typename RangeTp>
using BigEndianInputView = details::BitView<RangeTp, BigEndianInputBitIter>;

template <typename RangeTp>
using LittleEndianOutputView =
    details::BitView<RangeTp, LittleEndianOutputBitIter>;

template <typename RangeTp>
using BigEndianOutputView = details::BitView<RangeTp, BigEndianOutputBitIter>;

}  // namespace ranges

namespace views {

using LittleEndianInputAdaptorClosure =
    details::BitViewAdaptorClosure<ranges::LittleEndianInputView>;

using BigEndianInputAdaptorClosure =
    details::BitViewAdaptorClosure<ranges::BigEndianInputView>;

using LittleEndianOutputAdaptorClosure =
    details::BitViewAdaptorClosure<ranges::LittleEndianOutputView>;

using BigEndianOutputAdaptorClosure =
    details::BitViewAdaptorClosure<ranges::BigEndianOutputView>;

inline constexpr LittleEndianInputAdaptorClosure LittleEndianInput{};

inline constexpr BigEndianInputAdaptorClosure BigEndianInput{};

inline constexpr LittleEndianOutputAdaptorClosure LittleEndianOutput{};

inline constexpr BigEndianOutputAdaptorClosure BigEndianOutput{};

}  // namespace views

}  // namespace koda

#include <koda/utils/bit_iterator.tpp>
