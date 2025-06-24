#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/direct_decoder.hpp>
#include <koda/coders/lzss/lzss_intermediate_token.hpp>
#include <koda/collections/fuzed_dictionary_and_buffer.hpp>
#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/concepts.hpp>

namespace koda {

template <std::integral Token = uint8_t,
          SizeAwareDecoder<LzssIntermediateToken<Token>> AuxiliaryDecoder =
              DirectDecoder<LzssIntermediateToken<Token>>,
          typename Allocator = std::allocator<Token>>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
class LzssDecoder
    : public DecoderInterface<Token,
                              LzssDecoder<Token, AuxiliaryDecoder, Allocator>> {
   public:
    constexpr explicit LzssDecoder(
        size_t dictionary_size, size_t look_ahead_size,
        AuxiliaryDecoder auxiliary_decoder,
        std::optional<size_t> cyclic_buffer_size = std::nullopt,
        const Allocator& allocator = Allocator{});

    constexpr explicit LzssDecoder(
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
    using IMToken = LzssIntermediateToken<Token>;

    template <std::ranges::output_range<Token> RangeTp>
    class SlidingDecoderView;

    struct FusedDictAndBufferInfo {
        size_t dictionary_size;
        size_t look_ahead_size;
        std::optional<size_t> cyclic_buffer_size;
        [[no_unique_address]] Allocator allocator;
    };

    std::variant<FusedDictionaryAndBuffer<Token>, FusedDictAndBufferInfo>
        dictionary_and_buffer_;
    [[no_unique_address]] AuxiliaryDecoder auxiliary_decoder_;

    constexpr void LoadFusedDict();
};

}  // namespace koda

#include <koda/coders/lzss/lzss_decoder.tpp>
