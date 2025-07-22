#pragma once

#include <cassert>
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
concept BitInputIterator =
    std::input_iterator<Iter> && std::same_as<std::iter_value_t<Iter>, bool>;

template <class Iter>
concept BitOutputIterator = std::output_iterator<Iter, bool>;

template <class Range>
concept BitInputRange = std::ranges::input_range<Range> &&
                        BitInputIterator<std::ranges::iterator_t<Range>>;

template <class Range>
concept BitOutputRange = std::ranges::output_range<Range, bool> &&
                         BitOutputIterator<std::ranges::iterator_t<Range>>;

template <typename Iter>
concept BitIteratorUnderlyingInputIterator =
    std::input_iterator<Iter> && std::integral<std::iter_value_t<Iter>>;

template <typename Iter>
concept BitIteratorUnderlyingOutputIterator =
    std::output_iterator<Iter, std::iter_value_t<Iter>>;

template <typename Iter>
concept BitIteratorUnderlyingInputOrOutputIterator =
    BitIteratorUnderlyingInputIterator<Iter> ||
    BitIteratorUnderlyingOutputIterator<Iter>;

template <BitIteratorUnderlyingInputOrOutputIterator Iter>
class BitIteratorBase {
   public:
    explicit constexpr BitIteratorBase(Iter iterator) noexcept(
        std::is_nothrow_move_constructible_v<Iter>);

    explicit constexpr BitIteratorBase(Iter iterator, size_t position) noexcept(
        std::is_nothrow_move_constructible_v<Iter>);

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

    [[nodiscard]] static inline consteval size_t ByteLength() noexcept;

    // It seems that for derived types self is deduces as the derived type
    // itself which makes this increadibly useful
    [[nodiscard]] constexpr auto Rebind(this auto&& self, Iter other) noexcept;

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

template <BitIteratorUnderlyingInputIterator Iter>
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

    [[nodiscard]] constexpr bit operator*() const noexcept;

    constexpr LittleEndianInputBitIter& operator++() noexcept;

    [[nodiscard]] constexpr LittleEndianInputBitIter operator++(int) noexcept;

    [[nodiscard]] constexpr size_t Position() const noexcept;

   private:
    using TemporaryTp = std::iter_value_t<Iter>;

    static constexpr TemporaryTp kParityBitMask = 1;

    mutable bool should_fetch_ = true;
};

template <BitIteratorUnderlyingOutputIterator Iter>
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

    constexpr LittleEndianOutputBitIter& operator=(bit value) noexcept;

    [[nodiscard]] constexpr LittleEndianOutputBitIter& operator*(void) noexcept;

    constexpr LittleEndianOutputBitIter& operator++() noexcept;

    [[nodiscard]] constexpr LittleEndianOutputBitIter& operator++(int) noexcept;

    constexpr void Flush() noexcept;

    [[nodiscard]] constexpr size_t Position() const noexcept;
};

template <BitIteratorUnderlyingInputIterator Iter>
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

    [[nodiscard]] constexpr bit operator*() const noexcept;

    constexpr BigEndianInputBitIter& operator++() noexcept;

    [[nodiscard]] constexpr BigEndianInputBitIter operator++(int) noexcept;

    [[nodiscard]] constexpr size_t Position() const noexcept;

   private:
    using TemporaryTp = std::iter_value_t<Iter>;

    static constexpr TemporaryTp kSignBitMask =
        1 << (BitIteratorBase<Iter>::ByteLength() - 1);

    mutable bool should_fetch_ = true;
};

template <BitIteratorUnderlyingOutputIterator Iter>
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

    constexpr BigEndianOutputBitIter& operator=(bit value) noexcept;

    [[nodiscard]] constexpr BigEndianOutputBitIter& operator*() noexcept;

    constexpr BigEndianOutputBitIter& operator++() noexcept;

    [[nodiscard]] constexpr BigEndianOutputBitIter& operator++(int) noexcept;

    constexpr void Flush() noexcept;

    [[nodiscard]] constexpr size_t Position() const noexcept;
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

    [[nodiscard]] constexpr BitIteratorTp<iterator_type> begin() const;

    [[nodiscard]] constexpr auto end() const
        requires(!std::same_as<sentinel_type, std::default_sentinel_t>);

    [[nodiscard]] constexpr std::default_sentinel_t end() const
        requires std::same_as<sentinel_type, std::default_sentinel_t>;

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
    [[nodiscard]] constexpr auto operator()(Range&& range) const;
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

#include <koda/ranges/bit_iterator.tpp>
