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
    std::output_iterator<Iter, size_t> && requires(Iter iter) {
        { iter.Position() } -> std::same_as<size_t>;
    };

template <class Range>
concept BitInputRange = std::ranges::input_range<Range> &&
                        BitInputIterator<std::ranges::iterator_t<Range>>;

template <class Range>
concept BitOutputRange = std::ranges::output_range<Range, size_t> &&
                         BitOutputIterator<std::ranges::iterator_t<Range>>;

template <typename Tp, typename Up>
concept SameSize = (sizeof(Tp) == sizeof(Up));

template <typename Iter>
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

    [[nodiscard]] static inline consteval size_t ByteLength() noexcept {
        return CHAR_BIT * sizeof(std::iter_value_t<Iter>);
    }

    [[nodiscard]] constexpr bit value() const noexcept;

    constexpr void IncrementLittleEndianess() noexcept;

    constexpr void IncrementBigEndianess() noexcept;

    [[nodiscard]] constexpr size_t Position() const noexcept {
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

    [[nodiscard]] static constexpr InputBitIteratorSource
    MakeLittleEndianSource(Iter iter) noexcept(
        std::is_nothrow_move_constructible_v<Iter>) {
        return InputBitIteratorSource{std::move(iter), /*bit_iter*/ 0};
    }

    [[nodiscard]] static constexpr InputBitIteratorSource MakeBigEndianSource(
        Iter iter) noexcept(std::is_nothrow_move_constructible_v<Iter>) {
        return InputBitIteratorSource{std::move(iter),
                                      /*bit_iter*/ ByteLength() - 1};
    }

    constexpr ~InputBitIteratorSource() noexcept(
        std::is_nothrow_destructible_v<Iter>) = default;

   private:
    constexpr explicit InputBitIteratorSource(
        Iter iter,
        size_t bit_iter) noexcept(std::is_nothrow_move_constructible_v<Iter>)
        : iter_{std::move(iter)}, bit_iter_{bit_iter} {}

    Iter iter_;
    size_t bit_iter_;
};

template <typename Iter>
[[nodiscard]] constexpr InputBitIteratorSource<Iter>
MakeLittleEndianInputSource(Iter iter) {
    return InputBitIteratorSource<Iter>::MakeLittleEndianSource(
        std::move(iter));
}

template <typename Iter>
[[nodiscard]] constexpr InputBitIteratorSource<Iter> MakeBigEndianInputSource(
    Iter iter) {
    return InputBitIteratorSource<Iter>::MakeBigEndianSource(std::move(iter));
}

template <typename Iter>
class OutputBitIteratorSource {
   public:
    using bit = bool;

    constexpr OutputBitIteratorSource(
        const OutputBitIteratorSource&
            other) noexcept(std::is_nothrow_copy_constructible_v<Iter>) =
        default;
    constexpr OutputBitIteratorSource(OutputBitIteratorSource&& other) noexcept(
        std::is_nothrow_move_constructible_v<Iter>) = default;

    constexpr OutputBitIteratorSource&
    operator=(const OutputBitIteratorSource& other) noexcept(
        std::is_nothrow_copy_assignable_v<Iter>) = default;
    constexpr OutputBitIteratorSource&
    operator=(OutputBitIteratorSource&& other) noexcept(
        std::is_nothrow_move_assignable_v<Iter>) = default;

    [[nodiscard]] static inline consteval size_t ByteLength() noexcept {
        return CHAR_BIT * sizeof(std::iter_value_t<Iter>);
    }

    constexpr void SaveValueLittleEndian(bit value) noexcept;

    constexpr void SaveValueBigEndian(bit value) noexcept;

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return bit_iter_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        OutputBitIteratorSource const& left,
        OutputBitIteratorSource const& right) noexcept {
        return (left.iter_ == right.iter_) &&
               (left.bit_iter_ == right.bit_iter_);
    }

    [[nodiscard]] friend constexpr bool operator==(
        OutputBitIteratorSource const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return left.iter_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        OutputBitIteratorSource const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return sentinel == right.iter_;
    }

    [[nodiscard]] static constexpr OutputBitIteratorSource
    MakeLittleEndianSource(Iter iter) noexcept(
        std::is_nothrow_move_constructible_v<Iter>) {
        return OutputBitIteratorSource{std::move(iter), /*bit_iter*/ 0};
    }

    [[nodiscard]] static constexpr OutputBitIteratorSource MakeBigEndianSource(
        Iter iter) noexcept(std::is_nothrow_move_constructible_v<Iter>) {
        return OutputBitIteratorSource{std::move(iter),
                                       /*bit_iter*/ ByteLength() - 1};
    }

    constexpr ~OutputBitIteratorSource() noexcept(
        std::is_nothrow_destructible_v<Iter>) = default;

   private:
    constexpr explicit OutputBitIteratorSource(
        Iter iter,
        size_t bit_iter) noexcept(std::is_nothrow_move_constructible_v<Iter>)
        : iter_{std::move(iter)}, bit_iter_{bit_iter} {}

    std::iter_value_t<Iter> temporary_;
    Iter iter_;
    size_t bit_iter_;
};

template <typename Iter>
[[nodiscard]] constexpr OutputBitIteratorSource<Iter>
MakeLittleEndianOutputSource(Iter iter) {
    return OutputBitIteratorSource<Iter>::MakeLittleEndianSource(
        std::move(iter));
}

template <typename Iter>
[[nodiscard]] constexpr OutputBitIteratorSource<Iter> MakeBigEndianOutputSource(
    Iter iter) {
    return OutputBitIteratorSource<Iter>::MakeBigEndianSource(std::move(iter));
}

template <typename Iter>
class LittleEndianInputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr LittleEndianInputBitIter(
        InputBitIteratorSource<Iter>& source) noexcept
        : source_{std::addressof(source)} {}

    explicit constexpr LittleEndianInputBitIter() noexcept = default;

    [[nodiscard]] constexpr bit operator*() const noexcept {
        return source_->value();
    }

    constexpr LittleEndianInputBitIter& operator++() noexcept {
        source_->IncrementLittleEndianess();
        return *this;
    }

    [[nodiscard]] constexpr LittleEndianInputBitIter operator++(int) noexcept {
        source_->IncrementLittleEndianess();
        return *this;
    }

    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianInputBitIter const& left,
        LittleEndianInputBitIter const& right) noexcept {
        return *left.source_ == *right.source_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianInputBitIter const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return !left.source_ || *left.source_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        LittleEndianInputBitIter const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return !right.source_ || sentinel == *right.source_;
    }

    [[nodiscard]] static inline consteval size_t ByteLength() noexcept {
        return InputBitIteratorSource<Iter>::ByteLength();
    }

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return source_->Position();
    }

   private:
    InputBitIteratorSource<Iter>* source_ = nullptr;
};

template <typename Iter>
class LittleEndianOutputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr LittleEndianOutputBitIter(
        OutputBitIteratorSource<Iter>& source) noexcept
        : source_{std::addressof(source)} {}

    explicit constexpr LittleEndianOutputBitIter() noexcept = default;

    constexpr LittleEndianOutputBitIter& operator=(bit value) noexcept {
        source_->SaveValueLittleEndian(value);
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

    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianOutputBitIter const& left,
        LittleEndianOutputBitIter const& right) noexcept {
        return *left.source_ == *right.source_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        LittleEndianOutputBitIter const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return *left.source_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        LittleEndianOutputBitIter const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return sentinel == *right.source_;
    }

    [[nodiscard]] static inline consteval size_t ByteLength() noexcept {
        return OutputBitIteratorSource<Iter>::ByteLength();
    }

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return source_->Position();
    }

   private:
    OutputBitIteratorSource<Iter>* source_ = nullptr;
};

template <typename Iter>
class BigEndianInputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr BigEndianInputBitIter(
        InputBitIteratorSource<Iter>& source) noexcept
        : source_{std::addressof(source)} {}

    explicit constexpr BigEndianInputBitIter() noexcept = default;

    [[nodiscard]] constexpr bit operator*() const noexcept {
        return source_->value();
    }

    constexpr BigEndianInputBitIter& operator++() noexcept {
        source_->IncrementBigEndianess();
        return *this;
    }

    [[nodiscard]] constexpr BigEndianInputBitIter operator++(int) noexcept {
        source_->IncrementBigEndianess();
        return *this;
    }

    [[nodiscard]] friend constexpr bool operator==(
        BigEndianInputBitIter const& left,
        BigEndianInputBitIter const& right) noexcept {
        return *left.source_ == *right.source_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        BigEndianInputBitIter const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return !left.source_ || *left.source_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        BigEndianInputBitIter const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return !right.source_ || sentinel == *right.source_;
    }

    [[nodiscard]] static inline consteval size_t ByteLength() noexcept {
        return InputBitIteratorSource<Iter>::ByteLength();
    }

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return source_->Position();
    }

   private:
    InputBitIteratorSource<Iter>* source_ = nullptr;
};

template <typename Iter>
class BigEndianOutputBitIter {
   public:
    using bit = bool;
    using value_type = bit;
    using difference_type = std::ptrdiff_t;

    explicit constexpr BigEndianOutputBitIter(
        OutputBitIteratorSource<Iter>& source) noexcept
        : source_{std::addressof(source)} {}

    explicit constexpr BigEndianOutputBitIter() noexcept = default;

    constexpr BigEndianOutputBitIter& operator=(bit value) noexcept {
        source_->SaveValueBigEndian(value);
        return *this;
    }

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
        return *left.source_ == right.source_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        BigEndianOutputBitIter const& left,
        std::default_sentinel_t sentinel) noexcept
        requires(WeaklyEqualityComparable<Iter, std::default_sentinel_t>)
    {
        return *left.source_ == sentinel;
    }

    [[nodiscard]] friend constexpr bool operator==(
        std::default_sentinel_t sentinel,
        BigEndianOutputBitIter const& right) noexcept
        requires(WeaklyEqualityComparable<std::default_sentinel_t, Iter>)
    {
        return sentinel == *right.source_;
    }

    [[nodiscard]] static inline consteval size_t ByteLength() noexcept {
        return OutputBitIteratorSource<Iter>::ByteLength();
    }

    [[nodiscard]] constexpr size_t Position() const noexcept {
        return source_->Position();
    }

   private:
    OutputBitIteratorSource<Iter>* source_;
};

template <typename Range>
class LittleEndianInputBitRangeWrapper {
    using BeginSource =
        InputBitIteratorSource<std::ranges::iterator_t<const Range>>;
    using EndSource =
        InputBitIteratorSource<std::ranges::sentinel_t<const Range>>;

   public:
    using iterator_type =
        LittleEndianInputBitIter<std::ranges::iterator_t<const Range>>;
    using sentinel_type =
        LittleEndianInputBitIter<std::ranges::sentinel_t<const Range>>;

    constexpr explicit LittleEndianInputBitRangeWrapper(const Range& range)
        : begin_{BeginSource::MakeLittleEndianSource(
              std::ranges::begin(range))},
          end_{EndSource::MakeLittleEndianSource(std::ranges::end(range))} {}

    [[nodiscard]] constexpr iterator_type begin() {
        return iterator_type{begin_};
    }

    [[nodiscard]] constexpr sentinel_type end() { return sentinel_type{end_}; }

   private:
    BeginSource begin_;
    EndSource end_;
};

template <typename Range>
    requires std::same_as<std::ranges::sentinel_t<Range>,
                          std::default_sentinel_t>
class LittleEndianInputBitRangeWrapper<Range> {
    using BeginSource =
        InputBitIteratorSource<std::ranges::iterator_t<const Range>>;

   public:
    using iterator_type =
        LittleEndianInputBitIter<std::ranges::iterator_t<const Range>>;

    constexpr explicit LittleEndianInputBitRangeWrapper(const Range& range)
        : begin_{
              BeginSource::MakeLittleEndianSource(std::ranges::begin(range))} {}

    [[nodiscard]] constexpr iterator_type begin() {
        return iterator_type{begin_};
    }

    [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept {
        return std::default_sentinel;
    }

   private:
    BeginSource begin_;
};

template <typename Range>
class BigEndianInputBitRangeWrapper {
    using BeginSource =
        InputBitIteratorSource<std::ranges::iterator_t<const Range>>;
    using EndSource =
        InputBitIteratorSource<std::ranges::sentinel_t<const Range>>;

   public:
    using iterator_type =
        BigEndianInputBitIter<std::ranges::iterator_t<const Range>>;
    using sentinel_type =
        BigEndianInputBitIter<std::ranges::sentinel_t<const Range>>;

    constexpr explicit BigEndianInputBitRangeWrapper(const Range& range)
        : begin_{BeginSource::MakeBigEndianSource(std::ranges::begin(range))},
          end_{EndSource::MakeBigEndianSource(std::ranges::end(range))} {}

    [[nodiscard]] constexpr iterator_type begin() {
        return iterator_type{begin_};
    }

    [[nodiscard]] constexpr sentinel_type end() { return sentinel_type{end_}; }

   private:
    BeginSource begin_;
    EndSource end_;
};

template <typename Range>
    requires std::same_as<std::ranges::sentinel_t<Range>,
                          std::default_sentinel_t>
class BigEndianInputBitRangeWrapper<Range> {
    using BeginSource =
        InputBitIteratorSource<std::ranges::iterator_t<const Range>>;

   public:
    using iterator_type =
        BigEndianInputBitIter<std::ranges::iterator_t<const Range>>;

    constexpr explicit BigEndianInputBitRangeWrapper(const Range& range)
        : begin_{BeginSource::MakeBigEndianSource(std::ranges::begin(range))} {}

    [[nodiscard]] constexpr iterator_type begin() {
        return iterator_type{begin_};
    }

    [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept {
        return std::default_sentinel;
    }

   private:
    BeginSource begin_;
};

template <typename Range>
class LittleEndianOutputBitRangeWrapper {
    using BeginSource = OutputBitIteratorSource<std::ranges::iterator_t<Range>>;
    using EndSource = OutputBitIteratorSource<std::ranges::sentinel_t<Range>>;

   public:
    using iterator_type =
        LittleEndianOutputBitIter<std::ranges::iterator_t<Range>>;
    using sentinel_type =
        LittleEndianOutputBitIter<std::ranges::sentinel_t<Range>>;

    constexpr explicit LittleEndianOutputBitRangeWrapper(Range&& range)
        : begin_{BeginSource::MakeLittleEndianSource(
              std::ranges::begin(range))},
          end_{EndSource::MakeLittleEndianSource(std::ranges::end(range))} {}

    [[nodiscard]] constexpr iterator_type begin() {
        return iterator_type{begin_};
    }

    [[nodiscard]] constexpr sentinel_type end() { return sentinel_type{end_}; }

   private:
    BeginSource begin_;
    EndSource end_;
};

template <typename Range>
    requires std::same_as<std::ranges::sentinel_t<Range>,
                          std::default_sentinel_t>
class LittleEndianOutputBitRangeWrapper<Range> {
    using BeginSource = OutputBitIteratorSource<std::ranges::iterator_t<Range>>;

   public:
    using iterator_type =
        LittleEndianOutputBitIter<std::ranges::iterator_t<Range>>;

    constexpr explicit LittleEndianOutputBitRangeWrapper(Range&& range)
        : begin_{
              BeginSource::MakeLittleEndianSource(std::ranges::begin(range))} {}

    [[nodiscard]] constexpr iterator_type begin() {
        return iterator_type{begin_};
    }

    [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept {
        return std::default_sentinel;
    }

   private:
    BeginSource begin_;
};

template <typename Range>
class BigEndianOutputBitRangeWrapper {
    using BeginSource = OutputBitIteratorSource<std::ranges::iterator_t<Range>>;
    using EndSource = OutputBitIteratorSource<std::ranges::sentinel_t<Range>>;

   public:
    using iterator_type =
        BigEndianOutputBitIter<std::ranges::iterator_t<Range>>;
    using sentinel_type =
        BigEndianOutputBitIter<std::ranges::sentinel_t<Range>>;

    constexpr explicit BigEndianOutputBitRangeWrapper(Range&& range)
        : begin_{BeginSource::MakeBigEndianSource(std::ranges::begin(range))},
          end_{EndSource::MakeBigEndianSource(std::ranges::end(range))} {}

    [[nodiscard]] constexpr iterator_type begin() {
        return iterator_type{begin_};
    }

    [[nodiscard]] constexpr sentinel_type end() { return sentinel_type{end_}; }

   private:
    BeginSource begin_;
    EndSource end_;
};

template <typename Range>
    requires std::same_as<std::ranges::sentinel_t<Range>,
                          std::default_sentinel_t>
class BigEndianOutputBitRangeWrapper<Range> {
    using BeginSource = OutputBitIteratorSource<std::ranges::iterator_t<Range>>;

   public:
    using iterator_type =
        BigEndianOutputBitIter<std::ranges::iterator_t<Range>>;

    constexpr explicit BigEndianOutputBitRangeWrapper(Range&& range)
        : begin_{BeginSource::MakeBigEndianSource(std::ranges::begin(range))} {}

    [[nodiscard]] constexpr iterator_type begin() {
        return iterator_type{begin_};
    }

    [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept {
        return std::default_sentinel;
    }

   private:
    BeginSource begin_;
};

}  // namespace koda

#include <koda/utils/bit_iterator.tpp>
