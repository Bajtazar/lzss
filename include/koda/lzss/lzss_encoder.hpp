#pragma once

#include <koda/coder.hpp>
#include <koda/collections/fuzed_dictionary_and_buffer.hpp>
#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/concepts.hpp>

#include <memory>
#include <optional>

namespace koda {

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

#include <koda/lzss/lzss_encoder.tpp>
