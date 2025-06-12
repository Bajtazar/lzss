#pragma once

#include <koda/utils/type_dummies.hpp>

#include <cinttypes>

namespace koda {

template <typename Derived>
class EncoderInterface {
   public:
    constexpr explicit EncoderInterface() noexcept = default;

    constexpr auto operator()(InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output) {
        auto updated_output =
            self().Flush(Encode(std::forward<decltype(input)>(input),
                                std::forward<decltype(output)>(output)));
        auto iter = std::ranges::begin(updated_output);
        iter.Flush();
        return std::ranges::subrange{iter, std::ranges::end(updated_output)};
    }

   private:
    constexpr Derived& self() noexcept { return *static_cast<Derived*>(this); }
};

template <typename Derived>
class DecoderInterface {
   public:
    constexpr explicit DecoderInterface() noexcept = default;

    constexpr auto operator()(InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output) {
        return self().Decode(std::forward<decltype(input)>(input),
                             std::forward<decltype(output)>(output));
    }

   private:
    constexpr Derived& self() noexcept { return *static_cast<Derived*>(this); }
};

template <typename EncoderTp, typename Tp>
concept Encoder = requires(EncoderTp encoder, DummyInputRange<Tp> input,
                           DummyBitOutputRange output) {
    { encoder.Encode(input, output) } -> BitOutputRange;
    { encoder.Flush(output) } -> BitOutputRange;
    { encoder(input, output) } -> BitOutputRange;
};

template <typename EncoderTp, typename Tp>
concept SizeAwareEncoder =
    Encoder<EncoderTp, Tp> && requires(EncoderTp encoder, Tp token) {
        { encoder.TokenBitSize(token) } -> std::same_as<float>;
    };

template <typename DecoderTp, typename Tp>
concept Decoder = requires(DecoderTp decoder, DummyBitInputRange input,
                           DummyOutputRange<Tp> output) {
    { decoder.Decode(input, output) } -> std::ranges::output_range<Tp>;
    { decoder(input, output) } -> std::ranges::output_range<Tp>;
};

template <typename DecoderTp, typename Tp>
concept SizeAwareDecoder =
    Decoder<DecoderTp, Tp> && requires(DecoderTp decoder, Tp token) {
        { decoder.TokenBitSize(token) } -> std::same_as<float>;
    };

}  // namespace koda
