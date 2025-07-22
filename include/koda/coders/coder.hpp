#pragma once

#include <koda/ranges/views.hpp>
#include <koda/utils/concepts.hpp>
#include <koda/utils/type_dummies.hpp>
#include <koda/utils/utils.hpp>

#include <cinttypes>

namespace koda {

template <typename Tp, std::input_iterator InputIterTp,
          std::sentinel_for<InputIterTp> InputSentinelTp,
          std::output_iterator<Tp> OutputIterTp,
          std::sentinel_for<OutputIterTp> OutputSentinelTp>
struct CoderResult {
    template <std::ranges::input_range InputRangeTp,
              std::ranges::output_range<Tp> OutputRangeTp>
    constexpr explicit CoderResult(InputRangeTp&& input, OutputRangeTp&& output)
        : input_range{AsSubrange(std::forward<decltype(input)>(input))},
          output_range{AsSubrange(std::forward<decltype(output)>(output))} {}

    template <std::ranges::output_range<Tp> OutputRangeTp>
    constexpr explicit CoderResult(InputIterTp input_iter,
                                   InputSentinelTp input_sentinel,
                                   OutputRangeTp&& output)
        : input_range{std::move(input_iter), std::move(input_sentinel)},
          output_range{AsSubrange(std::forward<decltype(output)>(output))} {}

    template <std::ranges::input_range InputRangeTp>
    constexpr explicit CoderResult(InputRangeTp&& input,
                                   OutputIterTp output_iter,
                                   OutputSentinelTp output_sentinel)
        : input_range{AsSubrange(std::forward<decltype(input)>(input))},
          output_range{std::move(output_iter), std::move(output_sentinel)} {}

    constexpr explicit CoderResult(InputIterTp input_iter,
                                   InputSentinelTp input_sentinel,
                                   OutputIterTp output_iter,
                                   OutputSentinelTp output_sentinel)
        : input_range{std::move(input_iter), std::move(input_sentinel)},
          output_range{std::move(output_iter), std::move(output_sentinel)} {}

    std::ranges::subrange<InputIterTp, InputSentinelTp> input_range;
    std::ranges::subrange<OutputIterTp, OutputSentinelTp> output_range;
};

template <typename InputRangeTp, typename OutputRangeTp>
CoderResult(InputRangeTp&&, OutputRangeTp&&)
    -> CoderResult<std::ranges::range_value_t<OutputRangeTp>,
                   std::ranges::iterator_t<InputRangeTp>,
                   std::ranges::sentinel_t<InputRangeTp>,
                   std::ranges::iterator_t<OutputRangeTp>,
                   std::ranges::sentinel_t<OutputRangeTp>>;

template <typename InputIterTp, typename InputSentTp,
          std::ranges::range OutputRangeTp>
CoderResult(InputIterTp, InputSentTp, OutputRangeTp&&)
    -> CoderResult<std::ranges::range_value_t<OutputRangeTp>, InputIterTp,
                   InputSentTp, std::ranges::iterator_t<OutputRangeTp>,
                   std::ranges::sentinel_t<OutputRangeTp>>;

template <std::ranges::range InputRangeTp, typename OutputIter,
          typename OutputSentTp>
CoderResult(InputRangeTp&&, OutputIter, OutputSentTp) -> CoderResult<
    std::iter_value_t<OutputIter>, std::ranges::iterator_t<InputRangeTp>,
    std::ranges::sentinel_t<InputRangeTp>, OutputIter, OutputSentTp>;

template <typename InputIterTp, typename InputSentTp, typename OutputIter,
          typename OutputSentTp>
CoderResult(InputIterTp, InputSentTp, OutputIter, OutputSentTp)
    -> CoderResult<std::iter_value_t<OutputIter>, InputIterTp, InputSentTp,
                   OutputIter, OutputSentTp>;

template <typename InputToken, typename Derived>
class EncoderInterface {
    static constexpr bool IsSizeAware =
        requires(Derived derived, InputToken token) {
            { derived.TokenBitSize(token) } -> std::same_as<float>;
        };

   public:
    constexpr EncoderInterface() noexcept = default;

    constexpr auto operator()(InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output) {
        auto [irange, orange] =
            self().Encode(std::forward<decltype(input)>(input),
                          std::forward<decltype(output)>(output));
        auto updated_output = self().Flush(std::move(orange));
        return CoderResult{std::move(irange), std::move(updated_output)};
    }

    constexpr auto operator()(size_t stream_length,
                              InputRange<InputToken> auto&& input,
                              BitOutputRange auto&& output) {
        return RemoveCountedIters(
            (*this)(input | views::Take(stream_length),
                    std::forward<decltype(output)>(output)));
    }

    constexpr auto EncodeN(size_t stream_length,
                           InputRange<InputToken> auto&& input,
                           BitOutputRange auto&& output) {
        return RemoveCountedIters(
            self().Encode(input | views::Take(stream_length),
                          std::forward<decltype(output)>(output)));
    }

    // calculates information
    constexpr float EstimateEncodedSize(InputRange<InputToken> auto&& input)
        requires IsSizeAware
    {
        float est_size = 0;
        for (auto&& token : input) {
            est_size += self().TokenBitSize(token);
        }
        return est_size;
    }

    // calculates information
    constexpr float EstimateAverageTokenSize(
        InputRange<InputToken> auto&& input)
        requires IsSizeAware
    {
        float est_size = 0;
        size_t counter = 0;
        for (auto&& token : input) {
            est_size += self().TokenBitSize(token);
            ++counter;
        }
        return est_size / counter;
    }

   private:
    constexpr Derived& self() noexcept { return *static_cast<Derived*>(this); }

    constexpr auto RemoveCountedIters(
        SpecializationOf<CoderResult> auto&& result) {
        if constexpr (std::ranges::contiguous_range<
                          decltype(result.input_range)>) {
            return std::forward<decltype(result)>(result);
        } else {
            return CoderResult{std::ranges::begin(result.input_range).base(),
                               std::ranges::end(result.input_range).base(),
                               std::move(result.output_range)};
        }
    }
};

template <typename InputToken, typename Derived>
class DecoderInterface {
   public:
    constexpr DecoderInterface() noexcept = default;

    constexpr auto operator()(
        BitInputRange auto&& input,
        std::ranges::output_range<InputToken> auto&& output) {
        return self().Decode(
            self().Initialize(std::forward<decltype(input)>(input)),
            std::forward<decltype(output)>(output));
    }

    constexpr auto operator()(
        size_t stream_length, BitInputRange auto&& input,
        std::ranges::output_range<InputToken> auto&& output) {
        return RemoveCountedIters((*this)(std::forward<decltype(input)>(input),
                                          output | views::Take(stream_length)));
    }

    constexpr auto DecodeN(
        size_t stream_length, BitInputRange auto&& input,
        std::ranges::output_range<InputToken> auto&& output) {
        return RemoveCountedIters(
            self().Decode(std::forward<decltype(input)>(input),
                          output | views::Take(stream_length)));
    }

   private:
    constexpr Derived& self() noexcept { return *static_cast<Derived*>(this); }

    constexpr auto RemoveCountedIters(
        SpecializationOf<CoderResult> auto&& result) {
        if constexpr (std::ranges::contiguous_range<
                          decltype(result.output_range)>) {
            return std::forward<decltype(result)>(result);
        } else {
            return CoderResult{std::move(result.input_range),
                               std::ranges::begin(result.output_range).base(),
                               std::ranges::end(result.output_range).base()};
        }
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
    Encoder<EncoderTp, Tp> &&
    requires(EncoderTp encoder, Tp token, DummyInputRange<Tp> input) {
        { encoder.TokenBitSize(token) } -> std::same_as<float>;
        { encoder.EstimateEncodedSize(input) } -> std::same_as<float>;
        { encoder.EstimateAverageTokenSize(input) } -> std::same_as<float>;
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

}  // namespace koda
