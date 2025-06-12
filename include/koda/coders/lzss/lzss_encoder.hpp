#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/direct_encoder.hpp>
#include <koda/coders/lzss/lzss_intermediate_token.hpp>
#include <koda/collections/fuzed_dictionary_and_buffer.hpp>
#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/concepts.hpp>

#include <concepts>
#include <memory>
#include <optional>
#include <variant>

namespace koda {

template <std::integral InputToken = uint8_t,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder =
              DirectEncoder<LzssIntermediateToken<InputToken>>,
          typename AllocatorTp = std::allocator<InputToken>>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
class LzssEncoder
    : public EncoderInterface<
          InputToken, LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>> {
   public:
    constexpr explicit LzssEncoder(
        size_t dictionary_size, size_t look_ahead_size,
        AuxiliaryEncoder auxiliary_encoder,
        std::optional<size_t> cyclic_buffer_size = std::nullopt,
        const AllocatorTp& allocator = AllocatorTp{});

    constexpr explicit LzssEncoder(
        size_t dictionary_size, size_t look_ahead_size,
        std::optional<size_t> cyclic_buffer_size = std::nullopt,
        const AllocatorTp& allocator = AllocatorTp{})
        requires std::is_default_constructible_v<AuxiliaryEncoder>;

    constexpr auto Encode(InputRange<InputToken> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

    [[nodiscard]] constexpr auto&& auxiliary_encoder(this auto&& self);

   private:
    using SequenceView =
        typename FusedDictionaryAndBuffer<InputToken>::SequenceView;
    using IMToken = LzssIntermediateToken<InputToken>;
    using Match = typename SearchBinaryTree<InputToken>::RepeatitionMarker;

    struct FusedDictAndBufferInfo {
        size_t dictionary_size;
        std::optional<size_t> cyclic_buffer_size;
    };

    std::variant<FusedDictionaryAndBuffer<InputToken>, FusedDictAndBufferInfo>
        dictionary_and_buffer_;
    SearchBinaryTree<InputToken> search_tree_;
    AuxiliaryEncoder auxiliary_encoder_;
    uint16_t match_count_ = 0;

    constexpr auto InitializeBuffer(InputRange<InputToken> auto&& input);

    constexpr auto EncodeData(InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output);

    constexpr auto FlushData(BitOutputRange auto&& output);

    constexpr auto EncodeTokenOrMatch(InputToken token, const Match& match,
                                      BitOutputRange auto&& output);

    constexpr auto PeformEncodigStep(FusedDictionaryAndBuffer<InputToken>& dict,
                                     SequenceView look_ahead,
                                     BitOutputRange auto&& output);

    constexpr void TryToRemoveStringFromSearchTree(
        FusedDictionaryAndBuffer<InputToken>& dict);
};

}  // namespace koda

#include <koda/coders/lzss/lzss_encoder.tpp>
