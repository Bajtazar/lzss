#pragma once

#include <koda/coders/coder.hpp>
#include <koda/utils/concepts.hpp>

namespace koda {

template <UnsignedIntegral Token>
class RiceDecoder : public DecoderInterface<Token, RiceDecoder<Token>> {
   public:
    using token_type = Token;

    constexpr explicit RiceDecoder(size_t order) noexcept;

    constexpr auto Decode(BitInputRange auto&& input,
                          std::ranges::output_range<Token> auto&& output);

    constexpr auto Initialize(BitInputRange auto&& input);

   private:
    Token token_{};
    size_t order_;
    uint8_t bits_ = 0;

    constexpr auto DecodeToken(auto out_iter, auto iter, const auto& sent);
};

}  // namespace koda

#include <koda/coders/rice/rice_decoder.tpp>
