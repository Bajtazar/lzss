#pragma once

namespace koda {

template <typename Iter>
[[nodiscard]] constexpr InputBitIteratorSource<Iter>::bit
InputBitIteratorSource<Iter>::value() const noexcept {
    return ((1 << bit_iter_) & (*iter_)) >> bit_iter_;
}

template <typename Iter>
constexpr void
InputBitIteratorSource<Iter>::IncrementLittleEndianess() noexcept {
    if (++bit_iter_ == ByteLength()) {
        ++iter_;
        bit_iter_ = 0;
    }
}

template <typename Iter>
constexpr void InputBitIteratorSource<Iter>::IncrementBigEndianess() noexcept {
    if (!(bit_iter_--)) {
        ++iter_;
        bit_iter_ = ByteLength() - 1;
    }
}

template <typename Iter>
constexpr void
InputBitIteratorSource<Iter>::SkipToNextByteLittleEndianess() noexcept {
    ++iter_;
    bit_iter_ = 0;
}

template <typename Iter>
constexpr void
InputBitIteratorSource<Iter>::SkipToNextByteBigEndianess() noexcept {
    ++iter_;
    bit_iter_ = ByteLength() - 1;
}

template <typename Iter>
[[nodiscard]] constexpr std::byte
InputBitIteratorSource<Iter>::ReadByteLittleEndianess() noexcept {
    bit_iter_ = 0;
    return static_cast<std::byte>(*iter_++);
}

template <typename Iter>
[[nodiscard]] constexpr std::byte
InputBitIteratorSource<Iter>::ReadByteBigEndianess() noexcept {
    bit_iter_ = ByteLength() - 1;
    return static_cast<std::byte>(*iter_++);
}

template <typename Iter>
constexpr LittleEndianOutputBitIter<Iter>&
LittleEndianOutputBitIter<Iter>::operator=(bit value) noexcept {
    uint8_t mask = 1 << bit_iter_;
    temporary_ = (~mask & temporary_) | (value << bit_iter_);
    if (++bit_iter_ == ByteLength()) {
        *iter_++ = temporary_;
        bit_iter_ = temporary_ = 0;
    }
    return *this;
}

template <typename Iter>
constexpr void LittleEndianOutputBitIter<Iter>::SkipToNextByte(void) noexcept {
    *iter_++ = temporary_;
    bit_iter_ = temporary_ = 0;
}

template <typename Iter>
constexpr void LittleEndianOutputBitIter<Iter>::SaveByte(
    uint8_t byte) noexcept {
    if (bit_iter_) {
        SkipToNextByte();
    }
    *iter_++ = byte;
}

template <typename Iter>
[[nodiscard]] constexpr std::byte BigEndianInputBitIter<Iter>::ReadByte(
    void) noexcept {
    bit_iter_ = ByteLength() - 1;
    return static_cast<std::byte>(*iter_++);
}

template <typename Iter>
constexpr BigEndianOutputBitIter<Iter>& BigEndianOutputBitIter<Iter>::operator=(
    bit value) noexcept {
    uint8_t mask = 1 << bit_iter_;
    temporary_ = (~mask & temporary_) | (value << bit_iter_);
    if (!(bit_iter_--)) {
        *iter_++ = temporary_;
        bit_iter_ = ByteLength() - 1;
        temporary_ = 0;
    }
    return *this;
}

template <typename Iter>
constexpr void BigEndianOutputBitIter<Iter>::SkipToNextByte(void) noexcept {
    *iter_++ = temporary_;
    bit_iter_ = ByteLength() - 1;
    temporary_ = 0;
}

template <typename Iter>
constexpr void BigEndianOutputBitIter<Iter>::SaveByte(uint8_t byte) noexcept {
    if (bit_iter_ != ByteLength() - 1) {
        SkipToNextByte();
    }
    *iter_++ = byte;
}

}  // namespace koda
