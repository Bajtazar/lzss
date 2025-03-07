#pragma once

namespace koda {

template <ByteInputIterator Iter>
constexpr LittleEndianInputBitIter<Iter>&
LittleEndianInputBitIter<Iter>::operator++(void) noexcept {
    if (++bitIter == byteLength()) {
        ++iter;
        bitIter = 0;
    }
    return *this;
}

template <ByteInputIterator Iter>
[[nodiscard]] constexpr std::byte LittleEndianInputBitIter<Iter>::readByte(
    void) noexcept {
    bitIter = 0;
    return static_cast<std::byte>(*iter++);
}

template <ByteOutputIterator Iter>
constexpr LittleEndianOutputBitIter<Iter>&
LittleEndianOutputBitIter<Iter>::operator=(bit value) noexcept {
    uint8 mask = 1 << bitIter;
    temporary = (~mask & temporary) | (value << bitIter);
    if (++bitIter == byteLength()) {
        *iter++ = temporary;
        bitIter = temporary = 0;
    }
    return *this;
}

template <ByteOutputIterator Iter>
constexpr void LittleEndianOutputBitIter<Iter>::skipToNextByte(void) noexcept {
    *iter++ = temporary;
    bitIter = temporary = 0;
}

template <ByteOutputIterator Iter>
constexpr void LittleEndianOutputBitIter<Iter>::saveByte(
    std::byte byte) noexcept {
    temporary = bitIter = 0;
    *iter++ = byte;
}

template <ByteInputIterator Iter>
constexpr BigEndianInputBitIter<Iter>& BigEndianInputBitIter<Iter>::operator++(
    void) noexcept {
    if (!(bitIter--)) {
        ++iter;
        bitIter = 7;
    }
    return *this;
}

template <ByteInputIterator Iter>
[[nodiscard]] constexpr std::byte BigEndianInputBitIter<Iter>::readByte(
    void) noexcept {
    bitIter = 7;
    return static_cast<std::byte>(*iter++);
}

template <ByteOutputIterator Iter>
constexpr BigEndianOutputBitIter<Iter>& BigEndianOutputBitIter<Iter>::operator=(
    bit value) noexcept {
    uint8 mask = 1 << bitIter;
    temporary = (~mask & temporary) | (value << bitIter);
    if (!(bitIter--)) {
        *iter++ = temporary;
        bitIter = 7;
        temporary = 0;
    }
    return *this;
}

template <ByteOutputIterator Iter>
constexpr void BigEndianOutputBitIter<Iter>::skipToNextByte(void) noexcept {
    *iter++ = temporary;
    bitIter = 7;
    temporary = 0;
}

template <ByteOutputIterator Iter>
constexpr void BigEndianOutputBitIter<Iter>::saveByte(std::byte byte) noexcept {
    temporary = 0;
    bitIter = 7;
    *iter++ = byte;
}

}  // namespace koda
