#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/collections/map.hpp>

namespace koda {

template <typename Token, typename Count, typename State = size_t>
class TansDecoder : public DecoderInterface<Token, TansDecoder<Token, Count>> {
   public:
    using token_type = Token;
    using asymetrical = void;

    constexpr explicit TansDecoder(
        const TansInitTable<Token, Count>& init_table);

    constexpr auto Initialize(BitInputRange auto&& input);

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

   private:
    using BitIter = BigEndianOutputBitIter<State*>;
    using BitRange = std::pair<BitIter, BitIter>;

    struct DecodingEntry {
        Token symbol;
        State next_state;
        uint8_t bit_count;
    };

    std::vector<DecodingEntry> decoding_table_;
    BitRange receiver_;
    State state_ = 0;
    State received_bits_[1]{};

    constexpr auto HandleDiracDelta(
        BitInputRange auto&& input,
        std::ranges::output_range<Token> auto&& output);

    constexpr auto SetReceiver(auto iter, const auto& sent);

    constexpr Token DecodeToken();

    static constexpr std::vector<DecodingEntry> BuildDecodingTable(
        const TansInitTable<Token, Count>& init_table);
};

}  // namespace koda

#include <koda/coders/tans/tans_decoder.tpp>
