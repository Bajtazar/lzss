#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/collections/map.hpp>

namespace koda {

template <typename Token, typename Count, typename State = size_t>
class TansEncoder : public EncoderInterface<Token, TansEncoder<Token, Count>> {
   public:
    using asymetrical = void;
    using token_type = Token;

    constexpr explicit TansEncoder(
        const TansInitTable<Token, Count>& init_table);

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    using SState = std::make_signed_t<State>;
    using BitIter = LittleEndianInputBitIter<const State*>;
    using BitRange = std::pair<BitIter, BitIter>;

    Map<Token, uint8_t> saturation_map_;
    Map<Token, SState> offset_map_;
    Map<Token, State> renorm_map_;
    std::vector<State> encoding_table_;
    BitRange emitter_;
    State state_;
    State emitted_bits_[1];
    uint8_t shift_;

    constexpr auto EncodeTokens(InputRange<Token> auto&& input, auto iter,
                                auto sentinel);

    constexpr void EncodeToken(const auto& token);

    constexpr void SetEmitter(Count bit_count);

    constexpr auto FlushEmitter(auto output_iter, const auto& output_sent);

    static constexpr Map<Token, uint8_t> BuildSaturationMap(
        const TansInitTable<Token, Count>& init_table);

    static constexpr Map<Token, State> BuildRenormalizationOffsetMap(
        const TansInitTable<Token, Count>& init_table,
        const Map<Token, uint8_t>& saturation_map);

    static constexpr Map<Token, SState> BuildStartOffsetMap(
        const TansInitTable<Token, Count>& init_table);

    static constexpr std::vector<State> BuildEncodingTable(
        const TansInitTable<Token, Count>& init_table,
        const Map<Token, SState>& start_offset_map);
};

}  // namespace koda

#include <koda/coders/tans/tans_encoder.tpp>
