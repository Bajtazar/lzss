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
        BitInputRange auto&& input,
        std::ranges::output_range<InputToken> auto&& output) {
        auto istream = self().Initialize(std::forward<decltype(input)>(input));
        return self().Decode(std::move(istream),
                             std::forward<decltype(output)>(output));
    }

    constexpr auto operator()(
        size_t stream_length, BitInputRange auto&& input,
        std::ranges::output_range<InputToken> auto&& output) {
        return (*this)(std::forward<decltype(input)>(input),
                       output | std::views::take(stream_length));
    }

    constexpr auto DecodeN(size_t stream_length, BitInputRange auto&& input,
                          std::ranges::output_range<InputToken> auto&& output) {
        return self().Decode(std::forward<decltype(input)>(input),
                             output | std::views::take(stream_length));
    }

   private:
    constexpr Derived& self() noexcept { return *static_cast<Derived*>(this); }
};

template <typename Tp, BitInputRange InputRange,
          std::ranges::output_range<Tp> OutputRange>
struct DecoderResult {
    InputRange input_range;
    OutputRange output_range;
};

template <typename InputRange, typename OutputRange>
DecoderResult(InputRange&& input_range, OutputRange&& output_range)
    -> DecoderResult<std::ranges::range_value_t<OutputRange>, InputRange,
                     OutputRange>;

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
        { decoder.Initialize(input) } -> BitInputRange;
        { decoder.Decode(input, output) } -> SpecializationOf<DecoderResult>;
        {
            decoder.DecodeN(stream_length, input, output)
        } -> SpecializationOf<DecoderResult>;
        { decoder(input, output) } -> SpecializationOf<DecoderResult>;
        {
            decoder(stream_length, input, output)
        } -> SpecializationOf<DecoderResult>;
    };

template <typename DecoderTp, typename Tp>
concept SizeAwareDecoder =
    Decoder<DecoderTp, Tp> && requires(DecoderTp decoder, Tp token) {
        { decoder.TokenBitSize(token) } -> std::same_as<float>;
    };

}  // namespace koda
