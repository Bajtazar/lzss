#pragma once

#include <koda/utils/concepts.hpp>
#include <koda/utils/type_dummies.hpp>

#include <cinttypes>

namespace koda {

template <typename InputToken, typename Derived>
class EncoderInterface {
   public:
    constexpr explicit EncoderInterface() noexcept = default;

    constexpr auto operator()(InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output) {
        auto updated_output =
            self().Flush(self().Encode(std::forward<decltype(input)>(input),
                                       std::forward<decltype(output)>(output)));
        auto iter = std::ranges::begin(updated_output);
        iter.Flush();
        return std::ranges::subrange{iter, std::ranges::end(updated_output)};
    }

   private:
    constexpr Derived& self() noexcept { return *static_cast<Derived*>(this); }
};

template <typename InputToken, typename Derived>
class DecoderInterface {
   public:
    constexpr explicit DecoderInterface() noexcept = default;

    constexpr auto operator()(
        BitInputRange auto&& input, size_t stream_length,
        std::ranges::output_range<InputToken> auto&& output) {
        auto [istream, init_size] =
            self().Initialize(std::forward<decltype(input)>(input));
        return self().Decode(std::move(istream), stream_length - init_size,
                             std::forward<decltype(output)>(output));
    }

   private:
    constexpr Derived& self() noexcept { return *static_cast<Derived*>(this); }
};

template <typename Range>
struct DecoderResult {
    using range_type = Range;

    Range range;
    size_t init_tokens;
};

template <typename Result>
concept DecoderInitializationResult =
    SpecializationOf<Result, DecoderResult> &&
    BitInputRange<typename Result::range_type>;

template <typename Result, typename Tp>
concept DecoderDecodingResult =
    SpecializationOf<Result, DecoderResult> &&
    std::ranges::output_range<typename Result::range_type, Tp>;

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
concept Decoder =
    requires(DecoderTp decoder, size_t stream_length, DummyBitInputRange input,
             DummyOutputRange<Tp> output) {
        { decoder.Initialize(input) } -> DecoderInitializationResult;
        {
            decoder.Decode(input, stream_length, output)
        } -> DecoderDecodingResult<Tp>;
        { decoder(input, stream_length, output) } -> DecoderDecodingResult<Tp>;
    };

template <typename DecoderTp, typename Tp>
concept SizeAwareDecoder =
    Decoder<DecoderTp, Tp> && requires(DecoderTp decoder, Tp token) {
        { decoder.TokenBitSize(token) } -> std::same_as<float>;
    };

}  // namespace koda
