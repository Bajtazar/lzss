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
constexpr void OutputBitIteratorSource<Iter>::SaveValueLittleEndian(bit value) noexcept
{
    std::iter_value_t<Iter> mask = 1 << bit_iter_;
    temporary_ = (~mask & temporary_) | (value << bit_iter_);
    if (++bit_iter_ == ByteLength()) {
        *iter_++ = temporary_;
        bit_iter_ = temporary_ = 0;
    }
}

template <typename Iter>
constexpr void OutputBitIteratorSource<Iter>::SaveValueBigEndian(bit value) noexcept
{
    std::iter_value_t<Iter> mask = 1 << bit_iter_;
    temporary_ = (~mask & temporary_) | (value << bit_iter_);
    if (++bit_iter_ == ByteLength()) {
        *iter_++ = temporary_;
        bit_iter_ = temporary_ = 0;
    }
}

}  // namespace koda
