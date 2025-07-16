#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/collections/map.hpp>
#include <koda/utils/utils.hpp>

#include <cassert>
#include <cmath>

namespace koda {

template <typename Token, typename Count>
class TansEncoder : public EncoderInterface<Token, TansEncoder<Token, Count>> {
   public:
    constexpr explicit TansEncoder(
        const TansInitTable<Token, Count>& init_table);

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    using BitIter = LittleEndianInputBitIter<const Count*>;
    using BitRange = std::pair<BitIter, BitIter>;
    using SCount = std::make_signed_t<Count>;
    using State = Count;

    Map<Token, uint8_t> saturation_map_;
    Map<Token, SCount> offset_map_;
    Map<Token, Count> renorm_map_;
    std::vector<Count> encoding_table_;
    BitRange emitter_;
    State state_;
    State emitted_bits_[1];

    constexpr auto EncodeTokens(InputRange<Token> auto&& input, auto iter,
                                auto sentinel);

    constexpr void EncodeToken(const auto& token);

    constexpr void SetEmitter(Count bit_count);

    constexpr auto FlushEmitter(auto output_iter, const auto& output_sent);

    static constexpr Map<Token, uint8_t> BuildSaturationMap(
        const TansInitTable<Token, Count>& init_table);

    static constexpr Map<Token, Count> BuildRenormalizationOffsetMap(
        const TansInitTable<Token, Count>& init_table,
        const Map<Token, uint8_t>& saturation_map);

    static constexpr Map<Token, SCount> BuildStartOffsetMap(
        const TansInitTable<Token, Count>& init_table);

    static constexpr std::vector<Count> BuildEncodingTable(
        const TansInitTable<Token, Count>& init_table,
        const Map<Token, SCount>& start_offset_map);
};

}  // namespace koda

#include <koda/coders/tans/tans_encoder.tpp>
