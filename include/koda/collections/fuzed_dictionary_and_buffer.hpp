#pragma once

#include <cinttypes>
#include <cstdlib>
#include <optional>
#include <string_view>
#include <vector>

namespace koda {

/// @brief Fused dictionary and buffer handler. Stores them in the circular
/// buffer in a way that minimizes a number of string copying operations. This
/// class is optimized for the string accesses via the get_buffer and
/// get_oldest_dictionary_full_match() methods.
///
/// Invariants of this structure
/// - Only additon of a new symbol can override an existing symbol
/// - Nearest symbols won't be pruned
/// - After AddEndSymbolToBuffer call the AddSymbolToBuffer cannot be called
///
/// Utilizes a special telomere structures at the beginning and the end of the
/// circular buffer in ordet to minimize a number of heap allocations and string
/// copying. This is done as follows (lets consider M as buffer_size and N as
/// the circular buffer size)
///
/// Firstly lets consider a partially filled dictionary. In that position the
/// entire dictionary is contigous as so its last M elements. Buffer is also
/// contiguous. Buffer grows from left to right and it feeds the dictionary with
/// pruned symbols. Dictionary takes those symbols but upon filling to the
/// desired length it also will start to prune it last symbols
///
/// +--------+----------+---------------+
/// |  dict  |  buffer  |               |
/// +------^-+----------+------^--------+
///    left telomere         right telomere
///
/// Telomeres are located M-1 symbols from the edges from the circular buffer.
/// Now lets consider that buffer traveled to the end of the circular buffer
///
/// +----------+--------------+---------+
/// |          |    dict      |  buffer |
/// +------^---+--------------+^--------+
///    left telomere         right telomere
///
/// Now if we want to push a new symbol to the beginning of the buffer then it
/// will have to be inserted at the beginning of the circular buffer since all
/// of the space on its left side has been utilized. But that would void
/// assumption that the buffer is contiguous. To tackle this problem the first
/// M-1 symbols from the buffer (last one is pruned so it is not taken into an
/// account) is copied on the begining of the circular buffer
///
/// +-------+--+--------------+---------+
/// | buffer|  |    dict      | old_buf |
/// +------^+--+--------------+^--------+
///    left telomere         right telomere
///
/// After copying to the new buffer the new symbol is appended. The contiguity
/// of the buffer is restored and the dictionary can still grow as before since
/// old_buf is still valid.
///
/// Now when the dictionary pruned symbol reaches the right telomere then any
/// additional prunning would invalidate its invariant that it last M elements
/// have to be contiguous. Since the old buf and the left telomere contains the
/// same symbols then only the pointer to the dictionary is moved from the right
/// telomere to the beginning of the circular buffer and all of the properties
/// are restored
///
/// If the traditional circular buffer would be used then it would
/// result in 2 * M symbols being copied each time the access would
/// occur since structure wouldn't guarantee contiguity. That would
/// result in total of 2MN symbol copied per buffer and 2N allocations
/// on heap being made. The current implementation optimizes it to
/// 0 heap allocations and 2M symbols copied per N operations. Symbols
/// are also copied in 2 * M size batches which allows for SIMD-optimized
/// copying
class FusedDictionaryAndBuffer {
   public:
    using SequenceView = std::basic_string_view<uint8_t>;

    constexpr explicit FusedDictionaryAndBuffer(
        size_t dictionary_size, SequenceView buffer,
        std::optional<size_t> cyclic_buffer_size = std::nullopt);

    constexpr FusedDictionaryAndBuffer(const FusedDictionaryAndBuffer&) =
        delete;
    constexpr FusedDictionaryAndBuffer(FusedDictionaryAndBuffer&&) noexcept =
        default;

    constexpr FusedDictionaryAndBuffer& operator=(
        const FusedDictionaryAndBuffer&) = delete;
    constexpr FusedDictionaryAndBuffer& operator=(FusedDictionaryAndBuffer&&) =
        delete;

    constexpr bool AddSymbolToBuffer(uint8_t symbol);

    constexpr bool AddEndSymbolToBuffer();

    [[nodiscard]] constexpr SequenceView get_buffer() const noexcept;

    [[nodiscard]] constexpr SequenceView get_oldest_dictionary_full_match()
        const noexcept;

    [[nodiscard]] constexpr size_t max_dictionary_size() const noexcept;

    [[nodiscard]] constexpr size_t max_buffer_size() const noexcept;

    [[nodiscard]] constexpr size_t dictionary_size() const noexcept;

    [[nodiscard]] constexpr size_t buffer_size() const noexcept;

   private:
    using Buffer = std::vector<uint8_t>;
    using BufferIter = typename std::vector<uint8_t>::iterator;

    Buffer cyclic_buffer_;
    size_t dictionary_size_;
    size_t current_dictionary_size_ = 0;
    size_t buffer_size_;
    BufferIter dictionary_iter_;
    BufferIter dictionary_sentinel_;
    BufferIter buffer_iter_;
    BufferIter buffer_sentinel_;
    BufferIter left_telomere_tag_;
    BufferIter right_telomere_tag_;

    constexpr void RelocateBuffer();

    constexpr bool SlideDictionary();

    static constexpr size_t CalculateCyclicBufferSize(
        size_t dictionary_size, size_t buffer_size,
        std::optional<size_t> cyclic_buffer_size);
};

}  // namespace koda

#include <koda/collections/fuzed_dictionary_and_buffer.tpp>
