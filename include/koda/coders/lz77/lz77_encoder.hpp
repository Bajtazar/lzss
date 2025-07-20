#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/coder_traits.hpp>
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

namespace details {

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
class Lz77EncoderBase {
   public:
    constexpr explicit Lz77EncoderBase(
        size_t dictionary_size, size_t look_ahead_size,
        AuxiliaryEncoder auxiliary_encoder,
        std::optional<size_t> cyclic_buffer_size = std::nullopt,
        const Allocator& allocator = Allocator{});

    constexpr explicit Lz77EncoderBase(
        size_t dictionary_size, size_t look_ahead_size,
        std::optional<size_t> cyclic_buffer_size = std::nullopt,
        const Allocator& allocator = Allocator{})
        requires std::is_default_constructible_v<AuxiliaryEncoder>;

    [[nodiscard]] constexpr auto&& auxiliary_encoder(this auto&& self);

   protected:
    using SequenceView = typename FusedDictionaryAndBuffer<Token>::SequenceView;
    using IMToken = Lz77IntermediateToken<Token>;
    using Match = typename SearchBinaryTree<Token>::RepeatitionMarker;
    using AuxTraits = CoderTraits<AuxiliaryEncoder>;

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

    constexpr auto FlushQueue(BitOutputRange auto&& output);

    constexpr auto EncodeIntermediateToken(IMToken&& token,
                                           BitOutputRange auto&& output);
};

}  // namespace details

template <std::integral Token = uint8_t,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder =
              DirectEncoder<Lz77IntermediateToken<Token>>,
          typename Allocator = std::allocator<Token>>
class Lz77Encoder;

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsSymetrical)
class Lz77Encoder<Token, AuxiliaryEncoder, Allocator>
    : public EncoderInterface<Token,
                              Lz77Encoder<Token, AuxiliaryEncoder, Allocator>>,
      private details::Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator> {
    using Base = details::Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>;

   public:
    using token_type = Token;

    using Base::Lz77EncoderBase;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

    using Base::auxiliary_encoder;

    friend class details::Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>;

   private:
    using SequenceView = Base::SequenceView;
    using IMToken = Base::IMToken;
    using Match = Base::Match;
    using AuxTraits = Base::AuxTraits;

    constexpr auto EncodeData(InputRange<Token> auto&& input,
                              BitOutputRange auto&& output);

    constexpr auto FlushData(BitOutputRange auto&& output);

    constexpr auto EncodeTokenOrMatch(SequenceView buffer, const Match& match,
                                      BitOutputRange auto&& output);

    constexpr auto PeformEncodigStep(FusedDictionaryAndBuffer<Token>& dict,
                                     SequenceView buffer,
                                     SequenceView look_ahead,
                                     BitOutputRange auto&& output);

    constexpr void TryToRemoveStringFromSearchTree(
        FusedDictionaryAndBuffer<Token>& dict);

    constexpr std::pair<SequenceView, SequenceView> GetBufferAndLookAhead(
        FusedDictionaryAndBuffer<Token>& dict) const;
};

template <std::integral Token,
          SizeAwareEncoder<Lz77IntermediateToken<Token>> AuxiliaryEncoder,
          typename Allocator>
    requires(CoderTraits<AuxiliaryEncoder>::IsAsymetrical)
class Lz77Encoder<Token, AuxiliaryEncoder, Allocator>
    : public EncoderInterface<Token,
                              Lz77Encoder<Token, AuxiliaryEncoder, Allocator>>,
      private details::Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator> {
    using Base = details::Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>;

   public:
    using token_type = Token;

    constexpr explicit Lz77Encoder(
        size_t dictionary_size, size_t look_ahead_size,
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

    using Base::auxiliary_encoder;

    friend class details::Lz77EncoderBase<Token, AuxiliaryEncoder, Allocator>;

   private:
    using SequenceView = Base::SequenceView;
    using IMToken = Base::IMToken;
    using Match = Base::Match;
    using AuxTraits = Base::AuxTraits;

    constexpr auto EncodeData(InputRange<Token> auto&& input,
                              BitOutputRange auto&& output);

    constexpr auto PopulateDictionary(InputRange<Token> auto&& input,
                                      FusedDictionaryAndBuffer<Token>& dict);

    constexpr auto PeformEncodigStep(FusedDictionaryAndBuffer<Token>& dict,
                                     SequenceView buffer,
                                     SequenceView look_ahead,
                                     BitOutputRange auto&& output);

    constexpr auto EncodeTokenOrMatch(FusedDictionaryAndBuffer<Token>& dict,
                                      SequenceView buffer, Match&& match,
                                      BitOutputRange auto&& output);

    constexpr std::pair<SequenceView, SequenceView> GetBufferAndLookAhead(
        FusedDictionaryAndBuffer<Token>& dict) const;

    constexpr void AddStringToSearchTree(FusedDictionaryAndBuffer<Token>& dict);

    constexpr auto FlushData(BitOutputRange auto&& output);
};

}  // namespace koda

#include <koda/coders/lz77/lz77_encoder.tpp>
