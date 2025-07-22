#pragma once

namespace koda {

template <BitIteratorUnderlyingInputOrOutputIterator Iter>
constexpr BitIteratorBase<Iter>::BitIteratorBase(Iter iterator) noexcept(
    std::is_nothrow_move_constructible_v<Iter>)
    : iter_{std::move(iterator)} {}

template <BitIteratorUnderlyingInputOrOutputIterator Iter>
constexpr BitIteratorBase<Iter>::BitIteratorBase(
    Iter iterator,
    size_t position) noexcept(std::is_nothrow_move_constructible_v<Iter>)
    : iter_{std::move(iterator)} {
    assert(position < this->ByteLength());
    this->bit_iter_ += position;
}

template <BitIteratorUnderlyingInputOrOutputIterator Iter>
[[nodiscard]] /*static*/ inline consteval size_t
BitIteratorBase<Iter>::ByteLength() noexcept {
    return sizeof(TemporaryTp) * CHAR_BIT;
}

template <BitIteratorUnderlyingInputOrOutputIterator Iter>
[[nodiscard]] constexpr auto BitIteratorBase<Iter>::Rebind(this auto&& self,
                                                           Iter other) {
    // should decay
    auto copy = self;
    copy.iter_ = other;
    return copy;
}

template <BitIteratorUnderlyingInputOrOutputIterator Iter>
[[nodiscard]] constexpr auto&& BitIteratorBase<Iter>::Base(this auto&& self) {
    return std::forward_like<decltype(self)>(self.iter_);
}

template <BitIteratorUnderlyingInputIterator Iter>
[[nodiscard]] constexpr LittleEndianInputBitIter<Iter>::bit
LittleEndianInputBitIter<Iter>::operator*() const noexcept {
    if (should_fetch_) {
        this->current_value_ = *this->iter_ >> this->bit_iter_;
        should_fetch_ = false;
    }
    return this->current_value_ & kParityBitMask;
}

template <BitIteratorUnderlyingInputIterator Iter>
constexpr LittleEndianInputBitIter<Iter>&
LittleEndianInputBitIter<Iter>::operator++() noexcept {
    if (++this->bit_iter_ == this->ByteLength()) {
        this->bit_iter_ = 0;
        should_fetch_ = true;
        ++this->iter_;
    } else {
        this->current_value_ >>= 1;
    }
    return *this;
}

template <BitIteratorUnderlyingInputIterator Iter>
[[nodiscard]] constexpr LittleEndianInputBitIter<Iter>
LittleEndianInputBitIter<Iter>::operator++(int) noexcept {
    auto temp = *this;
    ++(*this);
    return temp;
}

template <BitIteratorUnderlyingInputIterator Iter>
[[nodiscard]] constexpr size_t LittleEndianInputBitIter<Iter>::Position()
    const noexcept {
    return this->bit_iter_;
}

template <BitIteratorUnderlyingOutputIterator Iter>
constexpr LittleEndianOutputBitIter<Iter>&
LittleEndianOutputBitIter<Iter>::operator=(bit value) noexcept {
    this->current_value_ |= (value ? 1 : 0) << this->bit_iter_;
    if (++this->bit_iter_ == this->ByteLength()) {
        *(this->iter_)++ = this->current_value_;
        this->bit_iter_ = this->current_value_ = 0;
    }
    return *this;
}

template <BitIteratorUnderlyingOutputIterator Iter>
[[nodiscard]] constexpr LittleEndianOutputBitIter<Iter>&
LittleEndianOutputBitIter<Iter>::operator*(void) noexcept {
    return *this;
}

template <BitIteratorUnderlyingOutputIterator Iter>
constexpr LittleEndianOutputBitIter<Iter>&
LittleEndianOutputBitIter<Iter>::operator++() noexcept {
    return *this;
}

template <BitIteratorUnderlyingOutputIterator Iter>
[[nodiscard]] constexpr LittleEndianOutputBitIter<Iter>&
LittleEndianOutputBitIter<Iter>::operator++(int) noexcept {
    return *this;
}

template <BitIteratorUnderlyingOutputIterator Iter>
constexpr void LittleEndianOutputBitIter<Iter>::Flush() noexcept {
    if (this->bit_iter_) {
        *(this->iter_)++ = this->current_value_;
        this->bit_iter_ = this->current_value_ = 0;
    }
}

template <BitIteratorUnderlyingOutputIterator Iter>
[[nodiscard]] constexpr size_t LittleEndianOutputBitIter<Iter>::Position()
    const noexcept {
    return this->bit_iter_;
}

template <BitIteratorUnderlyingInputIterator Iter>
[[nodiscard]] constexpr BigEndianInputBitIter<Iter>::bit
BigEndianInputBitIter<Iter>::operator*() const noexcept {
    if (should_fetch_) {
        this->current_value_ = *this->iter_ << this->bit_iter_;
        should_fetch_ = false;
    }
    return this->current_value_ & kSignBitMask;
}

template <BitIteratorUnderlyingInputIterator Iter>
constexpr BigEndianInputBitIter<Iter>&
BigEndianInputBitIter<Iter>::operator++() noexcept {
    if (++this->bit_iter_ == this->ByteLength()) {
        this->bit_iter_ = 0;
        should_fetch_ = true;
        ++this->iter_;
    } else {
        this->current_value_ <<= 1;
    }
    return *this;
}

template <BitIteratorUnderlyingInputIterator Iter>
[[nodiscard]] constexpr BigEndianInputBitIter<Iter>
BigEndianInputBitIter<Iter>::operator++(int) noexcept {
    auto temp = *this;
    ++(*this);
    return temp;
}

template <BitIteratorUnderlyingInputIterator Iter>
[[nodiscard]] constexpr size_t BigEndianInputBitIter<Iter>::Position()
    const noexcept {
    return this->ByteLength() - 1 - this->bit_iter_;
}

template <BitIteratorUnderlyingOutputIterator Iter>
constexpr BigEndianOutputBitIter<Iter>& BigEndianOutputBitIter<Iter>::operator=(
    bit value) noexcept {
    this->current_value_ = (this->current_value_ << 1) | (value ? 1 : 0);
    if (++this->bit_iter_ == this->ByteLength()) {
        *(this->iter_)++ = this->current_value_;
        this->bit_iter_ = this->current_value_ = 0;
    }
    return *this;
}

template <BitIteratorUnderlyingOutputIterator Iter>
[[nodiscard]] constexpr BigEndianOutputBitIter<Iter>&
BigEndianOutputBitIter<Iter>::operator*() noexcept {
    return *this;
}

template <BitIteratorUnderlyingOutputIterator Iter>
constexpr BigEndianOutputBitIter<Iter>&
BigEndianOutputBitIter<Iter>::operator++() noexcept {
    return *this;
}

template <BitIteratorUnderlyingOutputIterator Iter>
[[nodiscard]] constexpr BigEndianOutputBitIter<Iter>&
BigEndianOutputBitIter<Iter>::operator++(int) noexcept {
    return *this;
}

template <BitIteratorUnderlyingOutputIterator Iter>
constexpr void BigEndianOutputBitIter<Iter>::Flush() noexcept {
    if (this->bit_iter_) {
        *(this->iter_)++ = this->current_value_
                           << (this->ByteLength() - 1 - this->bit_iter_);
        this->bit_iter_ = this->current_value_ = 0;
    }
}

template <BitIteratorUnderlyingOutputIterator Iter>
[[nodiscard]] constexpr size_t BigEndianOutputBitIter<Iter>::Position()
    const noexcept {
    return this->ByteLength() - 1 - this->bit_iter_;
}

namespace details {

template <typename RangeTp, template <typename> class BitIteratorTp>
[[nodiscard]] constexpr BitIteratorTp<
    typename BitView<RangeTp, BitIteratorTp>::iterator_type>
BitView<RangeTp, BitIteratorTp>::begin() const {
    return BitIteratorTp<iterator_type>{std::ranges::begin(range_)};
}

template <typename RangeTp, template <typename> class BitIteratorTp>
[[nodiscard]] constexpr auto BitView<RangeTp, BitIteratorTp>::end() const
    requires(
        !std::same_as<typename BitView<RangeTp, BitIteratorTp>::sentinel_type,
                      std::default_sentinel_t>)
{
    return BitIteratorTp<sentinel_type>{std::ranges::end(range_)};
}

template <typename RangeTp, template <typename> class BitIteratorTp>
[[nodiscard]] constexpr std::default_sentinel_t
BitView<RangeTp, BitIteratorTp>::end() const
    requires std::same_as<
        typename BitView<RangeTp, BitIteratorTp>::sentinel_type,
        std::default_sentinel_t>
{
    return std::default_sentinel;
}

template <template <typename> class BitViewTp>
template <std::ranges::viewable_range Range>
[[nodiscard]] constexpr auto BitViewAdaptorClosure<BitViewTp>::operator()(
    Range&& range) const {
    return BitViewTp{std::forward<Range>(range)};
}

}  // namespace details

}  // namespace koda
