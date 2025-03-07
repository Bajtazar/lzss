#pragma once

namespace koda {

template <ByteInputIterator Iter>
constexpr LittleEndianInputbit_iter_<Iter>&
LittleEndianInputbit_iter_<Iter>::operator++(void) noexcept {
    if (++bit_iter_ == byteLength()) {
        ++iter_;
        bit_iter_ = 0;
    }
    return *this;
}

template <ByteInputIterator Iter>
[[nodiscard]] constexpr std::byte LittleEndianInputbit_iter_<Iter>::ReadByte(
    void) noexcept {
    bit_iter_ = 0;
    return static_cast<std::byte>(*iter_++);
}

template <ByteOutputIterator Iter>
constexpr LittleEndianOutputbit_iter_<Iter>&
LittleEndianOutputbit_iter_<Iter>::operator=(bit value) noexcept {
    uint8 mask = 1 << bit_iter_;
    temporary_ = (~mask & temporary_) | (value << bit_iter_);
    if (++bit_iter_ == byteLength()) {
        *iter_++ = temporary_;
        bit_iter_ = temporary_ = 0;
    }
    return *this;
}

template <ByteOutputIterator Iter>
constexpr void LittleEndianOutputbit_iter_<Iter>::SkipToNextByte(
    void) noexcept {
    *iter_++ = temporary_;
    bit_iter_ = temporary_ = 0;
}

template <ByteOutputIterator Iter>
constexpr void LittleEndianOutputbit_iter_<Iter>::saveByte(
    std::byte byte) noexcept {
    temporary_ = bit_iter_ = 0;
    *iter_++ = byte;
}

template <ByteInputIterator Iter>
constexpr BigEndianInputbit_iter_<Iter>&
BigEndianInputbit_iter_<Iter>::operator++(void) noexcept {
    if (!(bit_iter_--)) {
        ++iter_;
        bit_iter_ = 7;
    }
    return *this;
}

template <ByteInputIterator Iter>
[[nodiscard]] constexpr std::byte BigEndianInputbit_iter_<Iter>::ReadByte(
    void) noexcept {
    bit_iter_ = 7;
    return static_cast<std::byte>(*iter_++);
}

template <ByteOutputIterator Iter>
constexpr BigEndianOutputbit_iter_<Iter>&
BigEndianOutputbit_iter_<Iter>::operator=(bit value) noexcept {
    uint8 mask = 1 << bit_iter_;
    temporary_ = (~mask & temporary_) | (value << bit_iter_);
    if (!(bit_iter_--)) {
        *iter_++ = temporary_;
        bit_iter_ = 7;
        temporary_ = 0;
    }
    return *this;
}

template <ByteOutputIterator Iter>
constexpr void BigEndianOutputbit_iter_<Iter>::SkipToNextByte(void) noexcept {
    *iter_++ = temporary_;
    bit_iter_ = 7;
    temporary_ = 0;
}

template <ByteOutputIterator Iter>
constexpr void BigEndianOutputbit_iter_<Iter>::saveByte(
    std::byte byte) noexcept {
    temporary_ = 0;
    bit_iter_ = 7;
    *iter_++ = byte;
}

}  // namespace koda
