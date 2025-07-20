#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/direct_encoder.hpp>
#include <koda/coders/lz77/lz77_intermediate_token.hpp>
#include <koda/collections/fused_dictionary_and_buffer.hpp>
#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/concepts.hpp>

#include <concepts>
#include <memory>
#include <optional>
#include <variant>

namespace koda {

template <std::integral Token = uint8_t,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder =
              DirectEncoder<Lz77IntermediateToken<Token>>,
          typename Allocator = std::allocator<Token>>
class Lz77Encoder
    : public EncoderInterface<Token,
                              Lz77Encoder<Token, AuxiliaryEncoder, Allocator>> {
   public:
    using token_type = Token;

    constexpr explicit Lz77Encoder(
        size_t dictionary_size, size_t lookahead_size,
        AuxiliaryEncoder auxiliary_encoder,
        std::optional<size_t> cyclic_buffer_size = std::nullopt,
        const Allocator& allocator = Allocator{});

    constexpr explicit Lz77Encoder(
        size_t dictionary_size, size_t look_ahead_size,
        std::optional<size_t> cyclic_buffer_size = std::nullopt,
        const Allocator& allocator = Allocator{})
        requires std::is_default_constructible_v<AuxiliaryEncoder>;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

    [[nodiscard]] constexpr auto&& auxiliary_encoder(this auto&& self);

   private:
    using SequenceView = typename FusedDictionaryAndBuffer<Token>::SequenceView;
    using IMToken = Lz77IntermediateToken<Token>;
    using Match = typename SearchBinaryTree<Token>::RepeatitionMarker;

    struct FusedDictAndBufferInfo {
        size_t dictionary_size;
        std::optional<size_t> cyclic_buffer_size;
    };

    std::variant<FusedDictionaryAndBuffer<Token>, FusedDictAndBufferInfo>
        dictionary_and_buffer_;
    SearchBinaryTree<Token> search_tree_;
    std::optional<IMToken> queued_token_ = std::nullopt;
    uint16_t match_count_ = 0;
    [[no_unique_address]] AuxiliaryEncoder auxiliary_encoder_;

    constexpr auto InitializeBuffer(InputRange<Token> auto&& input);

    constexpr auto EncodeData(InputRange<Token> auto&& input,
                              BitOutputRange auto&& output);

    constexpr auto FlushQueue(BitOutputRange auto&& output);

    constexpr auto FlushData(BitOutputRange auto&& output);

    constexpr auto EncodeTokenOrMatch(SequenceView buffer, const Match& match,
                                      BitOutputRange auto&& output);

    constexpr auto PeformEncodigStep(FusedDictionaryAndBuffer<Token>& dict,
                                     SequenceView buffer,
                                     SequenceView look_ahead,
                                     BitOutputRange auto&& output);

    constexpr auto EncodeIntermediateToken(IMToken&& token,
                                           BitOutputRange auto&& output);

    constexpr void TryToRemoveStringFromSearchTree(
        FusedDictionaryAndBuffer<Token>& dict);

    constexpr std::pair<SequenceView, SequenceView> GetBufferAndLookAhead(
        FusedDictionaryAndBuffer<Token>& dict) const;
};

}  // namespace koda

#include <koda/coders/lz77/lz77_encoder.tpp>
