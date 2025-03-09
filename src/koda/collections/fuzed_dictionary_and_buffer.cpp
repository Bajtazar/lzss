#include <koda/collections/fuzed_dictionary_and_buffer.hpp>

#include <cstring>
#include <format>
#include <stdexcept>

namespace koda {

FusedDictionaryAndBuffer::FusedDictionaryAndBuffer(
    size_t dictionary_size, SequenceView buffer,
    std::optional<size_t> cyclic_buffer_size)
    : cyclic_buffer_(CalculateCyclicBufferSize(dictionary_size, buffer.size(),
                                               cyclic_buffer_size),
                     0),
      dictionary_size_{dictionary_size},
      buffer_size_{buffer.size()},
      dictionary_iter_{cyclic_buffer_.begin()},
      dictionary_sentinel_{dictionary_iter_},
      buffer_iter_{cyclic_buffer_.begin()},
      buffer_sentinel_{std::next(buffer_iter_, buffer_size_)},
      left_telomere_tag_{std::next(cyclic_buffer_.begin(), buffer_size_)},
      right_telomere_tag_{std::prev(cyclic_buffer_.end(), buffer_size_)} {
    if (dictionary_size < buffer_size_) [[unlikely]] {
        throw std::logic_error{std::format(
            "Dictionary size ({}) cannot be smaller than buffer size ({})",
            dictionary_size, buffer_size_)};
    }
    std::memcpy(buffer_iter_.base(), buffer.data(), buffer.size());
}

void FusedDictionaryAndBuffer::AddSymbolToBuffer(uint8_t symbol) {}

void FusedDictionaryAndBuffer::AddEndSymbolToBuffer() {}

[[nodiscard]] size_t FusedDictionaryAndBuffer::dictionary_size()
    const noexcept {
    std::ptrdiff_t difference = dictionary_iter_ - dictionary_sentinel_;
    if (difference < 0) {
        return static_cast<size_t>(right_telomere_tag_ - dictionary_sentinel_ +
                                   dictionary_iter_ - cyclic_buffer_.begin());
    }
    return static_cast<size_t>(difference);
}

[[nodiscard]] size_t FusedDictionaryAndBuffer::buffer_size() const noexcept {
    // Buffer is always contiguous so it cannot be splitted into two
    // parts. In contrast dictionary can be
    [[assume(buffer_sentinel_ <= buffer_sentinel_)]];
    return static_cast<size_t>(buffer_iter_ - buffer_sentinel_);
}

[[nodiscard]] size_t FusedDictionaryAndBuffer::max_dictionary_size()
    const noexcept {
    return dictionary_size_;
}

[[nodiscard]] size_t FusedDictionaryAndBuffer::max_buffer_size()
    const noexcept {
    return buffer_size_;
}

/*static*/ size_t FusedDictionaryAndBuffer::CalculateCyclicBufferSize(
    size_t dictionary_size, size_t buffer_size,
    std::optional<size_t> cyclic_buffer_size) {
    if (cyclic_buffer_size) {
        if (*cyclic_buffer_size < (dictionary_size + 2 * buffer_size))
            [[unlikely]] {
            throw std::logic_error{std::format(
                "Given cyclic buffer size is too small, expected at least "
                "dictionary size + 2*buffer size ({}), got ({})",
                dictionary_size + 2 * buffer_size, *cyclic_buffer_size)};
        }
        return *cyclic_buffer_size;
    }
    return 2 * (dictionary_size + buffer_size) + buffer_size;
}

}  // namespace koda
