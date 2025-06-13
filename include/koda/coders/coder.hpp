#pragma once

#include <koda/utils/concepts.hpp>
#include <koda/utils/type_dummies.hpp>

#include <cinttypes>

namespace koda {

template <typename InputRange, typename OutputRange>
struct CoderResult {
    InputRange input_range;
    OutputRange output_range;
};

template <typename InputToken, typename Derived>
class EncoderInterface {
   public:
    constexpr EncoderInterface() noexcept = default;

    constexpr auto operator()(InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output) {
        auto [irange, orange] =
            self().Encode(std::forward<decltype(input)>(input),
                          std::forward<decltype(output)>(output));
        auto updated_output = self().Flush(std::move(orange));
        auto iter = std::ranges::begin(updated_output);
        iter.Flush();
        return CoderResult{
            std::move(irange),
            std::ranges::subrange{std::move(iter),
                                  std::ranges::end(updated_output)}};
    }

    constexpr auto operator()(size_t stream_length,
                              InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output) {
        return RemoveCountedIters(
            (*this)(input | std::views::take(stream_length),
                    std::forward<decltype(output)>(output)));
    }

    constexpr auto EncodeN(size_t stream_length,
                           InputRange<InputToken> auto&& input,
                           BitOutputRange auto&& output) {
        return RemoveCountedIters(
            self().Encode(input | std::views::take(stream_length),
                          std::forward<decltype(output)>(output)));
    }

   private:
    constexpr Derived& self() noexcept { return *static_cast<Derived*>(this); }

    template <InputRange<InputToken> InputRange, BitOutputRange OutputRange>
    constexpr auto RemoveCountedIters(
        CoderResult<InputRange, OutputRange>&& result) {
        return CoderResult{
            std::ranges::subrange{std::ranges::begin(result.input_range).base(),
                                  std::ranges::end(result.input_range).base()},
            std::move(result.output_range)};
    }
};

template <typename InputToken, typename Derived>
class DecoderInterface {
   public:
    constexpr DecoderInterface() noexcept = default;

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
        return RemoveCountedIters(
            (*this)(std::forward<decltype(input)>(input),
                    output | std::views::take(stream_length)));
    }

    constexpr auto DecodeN(
        size_t stream_length, BitInputRange auto&& input,
        std::ranges::output_range<InputToken> auto&& output) {
        return RemoveCountedIters(
            self().Decode(std::forward<decltype(input)>(input),
                          output | std::views::take(stream_length)));
    }

   private:
    constexpr Derived& self() noexcept { return *static_cast<Derived*>(this); }

    template <BitInputRange InputRange,
              std::ranges::output_range<InputToken> OutputRange>
    constexpr auto RemoveCountedIters(
        CoderResult<InputRange, OutputRange>&& result) {
        return CoderResult{std::move(result.input_range),
                           std::ranges::subrange{
                               std::ranges::begin(result.output_range).base(),
                               std::ranges::end(result.output_range).base()}};
    }
};

template <typename EncoderTp, typename Tp>
concept Encoder =
    requires(EncoderTp encoder, size_t stream_length, DummyInputRange<Tp> input,
             DummyBitOutputRange output) {
        { encoder.Encode(input, output) } -> SpecializationOf<CoderResult>;
        {
            encoder.EncodeN(stream_length, input, output)
        } -> SpecializationOf<CoderResult>;
        { encoder.Flush(output) } -> BitOutputRange;
        { encoder(input, output) } -> SpecializationOf<CoderResult>;
        {
            encoder(stream_length, input, output)
        } -> SpecializationOf<CoderResult>;
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
        { decoder.Decode(input, output) } -> SpecializationOf<CoderResult>;
        {
            decoder.DecodeN(stream_length, input, output)
        } -> SpecializationOf<CoderResult>;
        { decoder(input, output) } -> SpecializationOf<CoderResult>;
        {
            decoder(stream_length, input, output)
        } -> SpecializationOf<CoderResult>;
    };

template <typename DecoderTp, typename Tp>
concept SizeAwareDecoder =
    Decoder<DecoderTp, Tp> && requires(DecoderTp decoder, Tp token) {
        { decoder.TokenBitSize(token) } -> std::same_as<float>;
    };

}  // namespace koda
