#pragma once

#include <koda/coders/coder.hpp>

template <typename Tp>
struct Lz77DummyAuxEncoder
    : public koda::EncoderInterface<Tp, Lz77DummyAuxEncoder<Tp>> {
    using token_type = Tp;

    constexpr explicit Lz77DummyAuxEncoder() = default;

    std::vector<Tp> tokens = {};

    [[noreturn]] constexpr float TokenBitSize([[maybe_unused]] Tp token) const {
        throw std::runtime_error{"Unused expression!"};
    }

    constexpr auto Encode(koda::InputRange<Tp> auto&& input,
                          koda::BitOutputRange auto&& output) {
        tokens.insert_range(tokens.end(), std::forward<decltype(input)>(input));
        // Circumvent static analysis
        auto iter = std::ranges::begin(input);
        const auto sent = std::ranges::end(input);
        for (; iter != sent; ++iter);
        return koda::CoderResult{iter, sent,
                                 std::forward<decltype(output)>(output)};
    }

    constexpr auto Flush(koda::BitOutputRange auto&& output) {
        return std::forward<decltype(output)>(output);
    }
};

template <typename Tp>
class Lz77DummyAuxDecoder
    : public koda::DecoderInterface<Tp, Lz77DummyAuxDecoder<Tp>> {
   public:
    using token_type = Tp;

    constexpr explicit Lz77DummyAuxDecoder(std::vector<Tp> tokens = {})
        : tokens_{std::move(tokens)} {}

    [[noreturn]] constexpr float TokenBitSize([[maybe_unused]] Tp token) const {
        throw std::runtime_error{"Unused expression!"};
    }

    constexpr auto Initialize(koda::BitInputRange auto&& input) {
        return koda::AsSubrange(std::forward<decltype(input)>(input));
    }

    constexpr auto Decode(koda::BitInputRange auto&& input,
                          std::ranges::output_range<Tp> auto&& output) {
        auto token_iter = tokens_.begin();
        auto out_iter = std::ranges::begin(output);
        auto out_sent = std::ranges::end(output);
        for (; token_iter != tokens_.end() && out_iter != out_sent;
             ++token_iter, ++out_iter) {
            *out_iter = *token_iter;
        }

        tokens_.erase(tokens_.begin(), token_iter);

        return koda::CoderResult{std::move(input), std::move(out_iter),
                                 std::move(out_sent)};
    }

   private:
    std::vector<Tp> tokens_;
};
