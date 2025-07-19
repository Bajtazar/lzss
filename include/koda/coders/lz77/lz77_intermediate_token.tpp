#pragma once

namespace koda {

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
constexpr Lz77IntermediateToken<InputToken, PositionTp, LengthTp>::
    Lz77IntermediateToken(Symbol suffix_symbol, PositionTp match_position,
                          LengthTp match_length) noexcept
    : suffix_symbol_{suffix_symbol},
      match_position_{match_position},
      match_length_{match_length} {}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr Lz77IntermediateToken<InputToken, PositionTp,
                                              LengthTp>::Symbol
Lz77IntermediateToken<InputToken, PositionTp, LengthTp>::suffix_symbol()
    const noexcept {
    return suffix_symbol_;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr PositionTp Lz77IntermediateToken<
    InputToken, PositionTp, LengthTp>::match_position() const noexcept {
    return match_position_;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] constexpr LengthTp Lz77IntermediateToken<
    InputToken, PositionTp, LengthTp>::match_length() const noexcept {
    return match_length_;
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
/*static*/ constexpr auto
TokenTraits<Lz77IntermediateToken<InputToken, PositionTp, LengthTp>>::
    EncodeToken(TokenType token, BitOutputRange auto&& output) {
    auto pos_range = TokenTraits<PositionTp>::EncodeToken(
        token.match_position(), std::forward<decltype(output)>(output));
    auto len_range = TokenTraits<LengthTp>::EncodeToken(token.match_length(),
                                                        std::move(pos_range));
    return TokenTraits<InputToken>::EncodeToken(token.suffix_symbol(),
                                                std::move(len_range));
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] /*static*/ constexpr auto
TokenTraits<Lz77IntermediateToken<InputToken, PositionTp, LengthTp>>::
    DecodeToken(BitInputRange auto&& input) {
    auto [position, pos_range] = TokenTraits<PositionTp>::DecodeToken(
        std::forward<decltype(input)>(input));
    auto [length, len_range] =
        TokenTraits<LengthTp>::DecodeToken(std::move(pos_range));
    auto [symbol, sym_range] =
        TokenTraits<InputToken>::DecodeToken(std::move(len_range));

    return TokenTraitsDecodingResult{
        Lz77IntermediateToken<InputToken, PositionTp, LengthTp>{
            symbol, position, length},
        std::move(sym_range)};
}

template <std::integral InputToken, UnsignedIntegral PositionTp,
          UnsignedIntegral LengthTp>
[[nodiscard]] /*static*/ constexpr float TokenTraits<Lz77IntermediateToken<
    InputToken, PositionTp, LengthTp>>::TokenBitSize(TokenType token) {
    return TokenTraits<PositionTp>::TokenBitSize(token.match_position()) +
           TokenTraits<LengthTp>::TokenBitSize(token.match_length()) +
           TokenTraits<LengthTp>::TokenBitSize(token.suffix_symbol());
}

}  // namespace koda

namespace std {

template <typename Token>
constexpr auto formatter<koda::Lz77IntermediateToken<Token>>::parse(
    std::format_parse_context& ctx) {
    return ctx.begin();
}

template <typename Token>
auto formatter<koda::Lz77IntermediateToken<Token>>::format(
    const koda::Lz77IntermediateToken<Token>& obj,
    std::format_context& ctx) const {
    return std::format_to(
        ctx.out(), "Lz77IntermediateToken(symbol={}, position={}, length={})",
        obj.suffix_symbol(), obj.match_position(), obj.match_length());
}

}  // namespace std
