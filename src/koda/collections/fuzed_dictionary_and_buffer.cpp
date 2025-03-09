#include <koda/collections/fuzed_dictionary_and_buffer.hpp>

#include <format>
#include <stdexcept>

namespace koda {

FusedDictionaryAndBuffer::FusedDictionaryAndBuffer(
    size_t dictionary_size, size_t buffer_size,
    std::optional<size_t> cyclic_buffer_size) {}

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
