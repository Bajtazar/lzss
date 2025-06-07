#pragma once

#include <koda/coder.hpp>
#include <koda/collections/fuzed_dictionary_and_buffer.hpp>
#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/concepts.hpp>

#include <memory>
#include <optional>

namespace koda {

class [[nodiscard]] LzssIntermediateToken {
   public:
    struct RepeatitionMarker {
        size_t match_position;
        size_t match_length;
    };

    constexpr explicit LzssIntermediateToken(uint8_t symbol) noexcept;

    constexpr explicit LzssIntermediateToken(size_t match_position,
                                             size_t match_length) noexcept;

    [[nodiscard]] constexpr bool holds_symbol() const noexcept;

    [[nodiscard]] constexpr bool holds_marker() const noexcept;

    [[nodiscard]] constexpr std::optional<uint8_t> get_symbol() const noexcept;

    [[nodiscard]] constexpr std::optional<RepeatitionMarker> get_marker()
        const noexcept;

   private:
    bool holds_distance_match_pair_;
    // We don't need to manually call destructor on an active
    // member since both of them are trivially destructible
    union {
        uint8_t symbol_;
        RepeatitionMarker repeatition_marker_;
    };
};

template <std::integral InputToken = uint8_t,
          Encoder<LzssIntermediateToken> AuxiliaryEncoder>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken))
class LzssEncoder {
   public:
    constexpr explicit LzssEncoder(
        size_t dictionary_size,
        AuxiliaryEncoder auxiliary_encoder = {}) noexcept;

    constexpr void Encode(InputRange<InputToken> auto&& input,
                          BitOutputRange auto&& output);

    constexpr void Flush(BitOutputRange auto&& output);

    constexpr void operator()(InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output) const;

   private:
    FusedDictionaryAndBuffer<InputToken> dictionary_and_buffer_;
    SearchBinaryTree<InputToken> search_tree_;
    AuxiliaryEncoder auxiliary_encoder_;
};

}  // namespace koda

#include <koda/lzss/lzss_encoder.ipp>
