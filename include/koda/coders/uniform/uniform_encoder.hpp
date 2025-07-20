#pragma once

#include <koda/coders/coder.hpp>

#include <array>
#include <concepts>

namespace koda {

template <typename Token>
struct UniformEncoderTraits;

template <std::integral Token>
struct UniformEncoderTraits<Token> {
    [[nodiscard]] consteval size_t TokenMaxByteSize() noexcept;

    template <size_t Size>
    static constexpr uint8_t PopulateBuffer(std::array<uint8_t, Size>& array,
                                            const Token& token);
};

template <typename Token>
class UniformEncoder : public EncoderInterface<Token, UniformEncoder<Token>> {
   public:
    using token_type = Token;

    using Traits = TokenTraits<Token>;

    constexpr explicit UniformEncoder() noexcept = default;

    constexpr float TokenBitSize(Token token) const;

    constexpr auto Encode(InputRange<Token> auto&& input,
                          BitOutputRange auto&& output);

    constexpr auto Flush(BitOutputRange auto&& output);

   private:
    using BitIter = LittleEndianInputBitIter<const State*>;
    using BitRange = std::pair<BitIter, BitIter>;
    using Traits = UniformEncoderTraits<Token>;

    std::array<uint8_t, Traits::TokenMaxByteSize()> buffer_;
    BitRange emitter_;

    constexpr auto EncodeTokens(InputRange<Token> auto&& input, auto iter,
                                auto sentinel);

    constexpr auto FlushEmitter(auto output_iter, const auto& output_sent);

    constexpr void SetEmitter(const Token& token);
};

}  // namespace koda

#include <koda/coders/uniform/uniform_encoder.tpp>
