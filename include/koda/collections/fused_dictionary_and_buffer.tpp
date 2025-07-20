#pragma once

#include <koda/utils/formatted_exception.hpp>
#include <koda/utils/utils.hpp>

#include <algorithm>
#include <cstring>

namespace koda {

template <typename Tp, typename AllocatorTp>
template <std::ranges::sized_range BufferInitRangeTp>
constexpr FusedDictionaryAndBuffer<Tp, AllocatorTp>::FusedDictionaryAndBuffer(
    size_t dictionary_size, BufferInitRangeTp&& buffer,
    std::optional<size_t> cyclic_buffer_size, const AllocatorTp& allocator)
    : FusedDictionaryAndBuffer{dictionary_size, std::ranges::size(buffer),
                               std::move(cyclic_buffer_size), allocator} {
    std::advance(buffer_sentinel_, buffer_size_);
    // Distance used to return iterator to the beginning of the cyclic buffer if
    // it overflows right telomere
    std::ranges::copy(std::forward<BufferInitRangeTp>(buffer), buffer_iter_);
}

template <typename Tp, typename AllocatorTp>
constexpr FusedDictionaryAndBuffer<Tp, AllocatorTp>::FusedDictionaryAndBuffer(
    size_t dictionary_size, size_t buffer_size,
    std::optional<size_t> cyclic_buffer_size, const AllocatorTp& allocator)
    : cyclic_buffer_(CalculateCyclicBufferSize(dictionary_size, buffer_size,
                                               cyclic_buffer_size),
                     0, allocator),
      dictionary_size_{dictionary_size},
      buffer_size_{buffer_size},
      dictionary_iter_{cyclic_buffer_.begin()},
      dictionary_sentinel_{dictionary_iter_},
      buffer_iter_{cyclic_buffer_.begin()},
      buffer_sentinel_{cyclic_buffer_.begin()},
      // buffer can be in fact empty, then only dictionary is being used. Useful
      // for decoders
      left_telomere_tag_{std::next(cyclic_buffer_.begin(), buffer_size_ - 1)},
      right_telomere_tag_{std::prev(cyclic_buffer_.end(), buffer_size_ - 1)} {
    if (buffer_size_ < 1) [[unlikely]] {
        throw std::logic_error{"Buffer size has to be greater than 0"};
    }
    cyclic_buffer_wrap_ =
        -std::distance(cyclic_buffer_.begin(), right_telomere_tag_);
}

template <typename Tp, typename AllocatorTp>
constexpr bool FusedDictionaryAndBuffer<Tp, AllocatorTp>::AddSymbolToBuffer(
    ValueType symbol) {
    // This can only occur during the beginning of the decoding when the buffer
    // is not yet expanded so there is no point in even checking the need for
    // the memory reajustment
    if (std::distance(buffer_iter_, buffer_sentinel_) != buffer_size_) {
        *buffer_sentinel_++ = symbol;
        return false;
    }

    if (buffer_sentinel_ == cyclic_buffer_.end()) [[unlikely]] {
        RelocateBuffer();
    } else {
        ++buffer_iter_;
    }
    *buffer_sentinel_++ = symbol;

    if (!SlideDictionary()) {
        ++current_dictionary_size_;
        return false;
    }
    return true;
}

template <typename Tp, typename AllocatorTp>
constexpr bool
FusedDictionaryAndBuffer<Tp, AllocatorTp>::AddEndSymbolToBuffer() {
    /// Relocation is not needed since no symbol is appended
    if (buffer_iter_ != buffer_sentinel_) {
        ++buffer_iter_;
        SlideDictionary();
        return false;
    }

    // If buffer is empty then start to shrink the dictionary
    IncrementDictionaryIterator();
    --current_dictionary_size_;
    return true;
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr AllocatorTp
FusedDictionaryAndBuffer<Tp, AllocatorTp>::get_allocator() const {
    return cyclic_buffer_.get_allocator();
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr FusedDictionaryAndBuffer<Tp, AllocatorTp>::SequenceView
FusedDictionaryAndBuffer<Tp, AllocatorTp>::get_buffer() const noexcept {
    // Always contiguous
    return SequenceView{buffer_iter_, buffer_sentinel_};
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr FusedDictionaryAndBuffer<Tp, AllocatorTp>::SequenceView
FusedDictionaryAndBuffer<Tp, AllocatorTp>::get_oldest_dictionary_full_match()
    const noexcept {
    // Always contiguous
    return SequenceView{dictionary_iter_,
                        std::next(dictionary_iter_, this->max_buffer_size())};
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr FusedDictionaryAndBuffer<Tp, AllocatorTp>::SequenceView
FusedDictionaryAndBuffer<Tp, AllocatorTp>::get_sequence_at_relative_pos(
    size_t position, size_t length) const {
    CheckRelativePosCorrectness(position, length);

    auto sequence_iter = std::next(dictionary_iter_, position);
    auto sequence_sent = std::next(sequence_iter, length);
    // If sequence overflows the cyclic buffer then just wrap it back to the
    // left telomere
    if (sequence_sent > cyclic_buffer_.end()) {
        std::advance(sequence_iter, cyclic_buffer_wrap_);
        std::advance(sequence_sent, cyclic_buffer_wrap_);
    }
    return SequenceView{sequence_iter, sequence_sent};
}

template <typename Tp, typename AllocatorTp>
constexpr void FusedDictionaryAndBuffer<Tp, AllocatorTp>::RelocateBuffer() {
    // When end symbols are added then this class contract permits usage of
    // AddSymbolToBuffer method and thus buffer size won't be changed and so
    // when relocation is happening the buffer always has to have its max size
    MemoryCopy(cyclic_buffer_.begin(), right_telomere_tag_, buffer_size_ - 1);
    buffer_iter_ = cyclic_buffer_.begin();
    buffer_sentinel_ = left_telomere_tag_;
    // First element will be a freashly inserted symbol
}

template <typename Tp, typename AllocatorTp>
constexpr bool FusedDictionaryAndBuffer<Tp, AllocatorTp>::SlideDictionary() {
    // Follow buffer
    if (dictionary_sentinel_++ == cyclic_buffer_.end()) [[unlikely]] {
        dictionary_sentinel_ = left_telomere_tag_;
    }

    // Determine whether dictionary should prune it last symbol
    if (current_dictionary_size_ == dictionary_size_) [[likely]] {
        // Prune the element if last buffer_size - 1 symbols of the dictionary
        // are contiguous
        IncrementDictionaryIterator();
        return true;
    }
    return false;
}

template <typename Tp, typename AllocatorTp>
constexpr void
FusedDictionaryAndBuffer<Tp, AllocatorTp>::IncrementDictionaryIterator() {
    if (++dictionary_iter_ == right_telomere_tag_) [[unlikely]] {
        // Otherwise the first M-1 element of the cyclic buffer are same as
        // the last M-1 ones
        dictionary_iter_ = cyclic_buffer_.begin();
    }
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr size_t
FusedDictionaryAndBuffer<Tp, AllocatorTp>::dictionary_size() const noexcept {
    return current_dictionary_size_;
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr size_t
FusedDictionaryAndBuffer<Tp, AllocatorTp>::buffer_size() const noexcept {
    // Buffer is always contiguous so it cannot be splitted into two
    // parts. In contrast dictionary can be
    [[assume(buffer_sentinel_ >= buffer_iter_)]];
    return static_cast<size_t>(buffer_sentinel_ - buffer_iter_);
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr size_t FusedDictionaryAndBuffer<
    Tp, AllocatorTp>::max_dictionary_size() const noexcept {
    return dictionary_size_;
}

template <typename Tp, typename AllocatorTp>
[[nodiscard]] constexpr size_t
FusedDictionaryAndBuffer<Tp, AllocatorTp>::max_buffer_size() const noexcept {
    return buffer_size_;
}

template <typename Tp, typename AllocatorTp>
/*static*/ constexpr size_t
FusedDictionaryAndBuffer<Tp, AllocatorTp>::CalculateCyclicBufferSize(
    size_t dictionary_size, size_t buffer_size,
    std::optional<size_t> cyclic_buffer_size) {
    if (cyclic_buffer_size) {
        if (*cyclic_buffer_size < (dictionary_size + 2 * buffer_size - 1))
            [[unlikely]] {
            throw FormattedException{
                "Given cyclic buffer size is too small, expected at least "
                "dictionary size + 2*buffer size-1 ({}), got ({})",
                dictionary_size + 2 * buffer_size - 1, *cyclic_buffer_size};
        }
        return *cyclic_buffer_size;
    }
    return 4 * (dictionary_size + buffer_size);
}

#ifdef KODA_CHECKED_BUILD

template <typename Tp, typename AllocatorTp>
constexpr void
FusedDictionaryAndBuffer<Tp, AllocatorTp>::CheckRelativePosCorrectness(
    size_t position, size_t length) const {
    if (length > buffer_size_) [[unlikely]] {
        // Currently does not work since https://wg21.link/P3068R6 is not
        // implemented yet by any compiler!
        throw FormattedException{
            "Sequence (len={}) is longer than bufer (len={})!", length,
            buffer_size_};
    }
    if (position + length > (dictionary_size_ + buffer_size_)) [[unlikely]] {
        throw FormattedException{
            "Given position (pos={} + len={}) overflows the "
            "fused buffer length (len={})!",
            position, length, dictionary_size_ + buffer_size_};
    }
}

#endif  // KODA_CHECKED_BUILD

}  // namespace koda
