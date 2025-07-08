#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/direct_decoder.hpp>
#include <koda/coders/lz77/lz77_intermediate_token.hpp>
#include <koda/collections/fused_dictionary_and_buffer.hpp>
#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/concepts.hpp>

#include <optional>

namespace koda {

template <std::integral Token = uint8_t,
          SizeAwareDecoder<Lz77IntermediateToken<Token>> AuxiliaryDecoder =
              DirectDecoder<Lz77IntermediateToken<Token>>,
          typename Allocator = std::allocator<Token>>
class Lz77Decoder
    : public DecoderInterface<Token,
                              Lz77Decoder<Token, AuxiliaryDecoder, Allocator>> {
   public:
    constexpr explicit Lz77Decoder(
        size_t dictionary_size, size_t look_ahead_size,
        AuxiliaryDecoder auxiliary_decoder,
        std::optional<size_t> cyclic_buffer_size = std::nullopt,
        const Allocator& allocator = Allocator{});

    constexpr explicit Lz77Decoder(
        size_t dictionary_size, size_t look_ahead_size,
        std::optional<size_t> cyclic_buffer_size = std::nullopt,
        const Allocator& allocator = Allocator{})
        requires std::is_default_constructible_v<AuxiliaryDecoder>;

    constexpr auto Initialize(BitInputRange auto&& input);

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

    [[nodiscard]] constexpr auto&& auxiliary_decoder(this auto&& self);

   private:
    using SequenceView = typename FusedDictionaryAndBuffer<Token>::SequenceView;
    using IMToken = Lz77IntermediateToken<Token>;

    template <std::ranges::output_range<Token> RangeTp>
    class SlidingDecoderView;

    struct CachedSequence {
        size_t position;
        size_t length;
        Token suffix;
    };

    FusedDictionaryAndBuffer<Token> dictionary_;
    std::optional<CachedSequence> cached_sequence_ = std::nullopt;
    [[no_unique_address]] AuxiliaryDecoder auxiliary_decoder_;

    constexpr auto ProcessCachedSequence(
        std::ranges::output_range<Token> auto&& output);

    constexpr auto ProcessData(BitInputRange auto&& input,
                               std::ranges::output_range<Token> auto&& output);
};

}  // namespace koda

#include <koda/coders/lz77/lz77_decoder.tpp>
