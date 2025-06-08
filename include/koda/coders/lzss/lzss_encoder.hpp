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
#include <iostream>

namespace koda {

template <std::integral InputToken = uint8_t,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder =
              DirectEncoder<LzssIntermediateToken<InputToken>>,
          typename AllocatorTp = std::allocator<InputToken>>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
class LzssEncoder {
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

    constexpr void Encode(InputRange<InputToken> auto&& input,
                          BitOutputRange auto&& output);

    constexpr void Flush(BitOutputRange auto&& output);

    constexpr void operator()(InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output);

   private:
    using SequenceView =
        typename FusedDictionaryAndBuffer<InputToken>::SequenceView;
    using IMToken = LzssIntermediateToken<InputToken>;

    struct FusedDictAndBufferInfo {
        size_t dictionary_size;
        std::optional<size_t> cyclic_buffer_size;
    };

    std::variant<FusedDictionaryAndBuffer<InputToken>, FusedDictAndBufferInfo>
        dictionary_and_buffer_;
    SearchBinaryTree<InputToken> search_tree_;
    AuxiliaryEncoder auxiliary_encoder_;

    constexpr auto InitializeBuffer(InputRange<InputToken> auto& input);

    constexpr void EncodeData(InputRange<InputToken> auto& input,
                                    BitOutputRange auto& output);
};

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr void LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::Encode(
    InputRange<InputToken> auto&& input, BitOutputRange auto&& output) {
    if (std::holds_alternative<FusedDictAndBufferInfo>(
            dictionary_and_buffer_)) {
        auto processed_input = InitializeBuffer(input);
        return EncodeData(processed_input, output);
    }
    EncodeData(input, output);
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr auto
LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::InitializeBuffer(
    InputRange<InputToken> auto& input) {
    const size_t look_ahead_size = search_tree_.string_size();

    std::vector<InputToken> init_view{
        std::from_range, input | std::views::take(look_ahead_size)};

    auto [dict_size, cyclic_buffer_size] =
        std::get<FusedDictAndBufferInfo>(dictionary_and_buffer_);
    dictionary_and_buffer_ = FusedDictionaryAndBuffer{
        dict_size, SequenceView{init_view}, std::move(cyclic_buffer_size),
        search_tree_.get_allocator()};

    return input | std::views::drop(look_ahead_size);
}

template <std::integral InputToken,
          SizeAwareEncoder<LzssIntermediateToken<InputToken>> AuxiliaryEncoder,
          typename AllocatorTp>
    requires(sizeof(InputToken) <= sizeof(LzssIntermediateToken<InputToken>))
constexpr void LzssEncoder<InputToken, AuxiliaryEncoder, AllocatorTp>::EncodeData(
    InputRange<InputToken> auto& input,
    BitOutputRange auto& output) {
    [[assume(std::holds_alternative<FusedDictionaryAndBuffer<InputToken>>(dictionary_and_buffer_))]];

    auto& dict = std::get<FusedDictionaryAndBuffer<InputToken>>(dictionary_and_buffer_);
    for (const auto& token : input) {
        search_tree_.AddString(dict.get_buffer());
        dict.AddSymbolToBuffer(token);
        auto marker = search_tree_.FindMatch(dict.get_buffer());



        std::cout << token << " -> ";
        std::cout << marker.match_position << ": "
            << marker.match_length << " with sizes ";

        IMToken symbol_token{token};
        IMToken match_token{marker.match_position,
        marker.match_length};

        std::cout << auxiliary_encoder_.TokenBitSize(symbol_token)
            << " x " <<auxiliary_encoder_.TokenBitSize(match_token) << "\n";

    }


}

}  // namespace koda

#include <koda/coders/lzss/lzss_encoder.tpp>
