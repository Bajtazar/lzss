#pragma once

#include <cinttypes>
#include <cstdlib>
#include <optional>
#include <string_view>
#include <vector>

namespace koda {

class FusedDictionaryAndBuffer {
   public:
    using SequenceView = std::basic_string_view<uint8_t>;

    explicit FusedDictionaryAndBuffer(
        size_t dictionary_size, size_t buffer_size,
        std::optional<size_t> cyclic_buffer_size = std::nullopt);

    FusedDictionaryAndBuffer(const FusedDictionaryAndBuffer&) = delete;
    FusedDictionaryAndBuffer(FusedDictionaryAndBuffer&&) noexcept = default;

    FusedDictionaryAndBuffer& operator=(const FusedDictionaryAndBuffer&) =
        delete;
    FusedDictionaryAndBuffer& operator=(FusedDictionaryAndBuffer&&) = delete;

    void AddSymbolToBuffer(uint8_t symbol);

    void AddEndSymbolToBuffer();

    [[nodiscard]] SequenceView get_buffer() const noexcept;

    [[nodiscard]] SequenceView get_oldest_dictionary_full_match()
        const noexcept;

    [[nodiscard]] size_t max_dictionary_size() const noexcept;

    [[nodiscard]] size_t max_buffer_size() const noexcept;

    [[nodiscard]] size_t dictionary_size() const noexcept;

    [[nodiscard]] size_t buffer_size() const noexcept;

   private:
    using Buffer = std::vector<uint8_t>;
    using BufferIter = typename std::vector<uint8_t>::iterator;

    Buffer cyclic_buffer_;
    size_t dictionary_size_;
    size_t buffer_size_;
    BufferIter dictionary_iter_;
    BufferIter look_ahead_iter_;
    BufferIter left_telomere_tag_;
    BufferIter right_telomere_tag_;
};

}  // namespace koda
