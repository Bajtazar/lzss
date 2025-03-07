#pragma once

namespace koda {

template <ByteInputIterator Iter>
constexpr LittleEndianInputBitIter<Iter>&
LittleEndianInputBitIter<Iter>::operator++(void) noexcept {
    if (++bit_iter_ == ByteLength()) {
        ++iter_;
        bit_iter_ = 0;
    }
    return *this;
}

template <ByteInputIterator Iter>
[[nodiscard]] constexpr std::byte LittleEndianInputBitIter<Iter>::ReadByte(
    void) noexcept {
    bit_iter_ = 0;
    return static_cast<std::byte>(*iter_++);
}

template <ByteOutputIterator Iter>
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

template <ByteOutputIterator Iter>
constexpr void LittleEndianOutputBitIter<Iter>::SkipToNextByte(void) noexcept {
    *iter_++ = temporary_;
    bit_iter_ = temporary_ = 0;
}

template <ByteOutputIterator Iter>
constexpr void LittleEndianOutputBitIter<Iter>::SaveByte(
    uint8_t byte) noexcept {
    if (bit_iter_) {
        SkipToNextByte();
    }
    *iter_++ = byte;
}

template <ByteInputIterator Iter>
constexpr BigEndianInputBitIter<Iter>& BigEndianInputBitIter<Iter>::operator++(
    void) noexcept {
    if (!(bit_iter_--)) {
        ++iter_;
        bit_iter_ = 7;
    }
    return *this;
}

template <ByteInputIterator Iter>
[[nodiscard]] constexpr std::byte BigEndianInputBitIter<Iter>::ReadByte(
    void) noexcept {
    bit_iter_ = 7;
    return static_cast<std::byte>(*iter_++);
}

template <ByteOutputIterator Iter>
constexpr BigEndianOutputBitIter<Iter>& BigEndianOutputBitIter<Iter>::operator=(
    bit value) noexcept {
    uint8_t mask = 1 << bit_iter_;
    temporary_ = (~mask & temporary_) | (value << bit_iter_);
    if (!(bit_iter_--)) {
        *iter_++ = temporary_;
        bit_iter_ = 7;
        temporary_ = 0;
    }
    return *this;
}

template <ByteOutputIterator Iter>
constexpr void BigEndianOutputBitIter<Iter>::SkipToNextByte(void) noexcept {
    *iter_++ = temporary_;
    bit_iter_ = 7;
    temporary_ = 0;
}

template <ByteOutputIterator Iter>
constexpr void BigEndianOutputBitIter<Iter>::SaveByte(std::byte byte) noexcept {
    temporary_ = 0;
    bit_iter_ = 7;
    *iter_++ = byte;
}

}  // namespace koda
