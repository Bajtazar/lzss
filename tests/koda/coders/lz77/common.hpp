#pragma once

#include <koda/coders/coder.hpp>

namespace details {

template <typename Tp>
struct Lz77DummyAuxEncoderBase
    : public koda::EncoderInterface<Tp, Lz77DummyAuxEncoderBase<Tp>> {
    constexpr explicit Lz77DummyAuxEncoderBase() = default;

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
class Lz77DummyAuxDecoderBase
    : public koda::DecoderInterface<Tp, Lz77DummyAuxDecoderBase<Tp>> {
   public:
    using token_type = Tp;

    constexpr explicit Lz77DummyAuxDecoderBase(std::vector<Tp> tokens = {})
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

}  // namespace details

template <typename Tp, bool IsAsymetrical>
struct Lz77DummyAuxEncoder;

template <typename Tp>
struct Lz77DummyAuxEncoder<Tp, false>
    : public details::Lz77DummyAuxEncoderBase<Tp> {
    using details::Lz77DummyAuxEncoderBase<Tp>::Lz77DummyAuxEncoderBase;

    using token_type = Tp;
};

template <typename Tp>
struct Lz77DummyAuxEncoder<Tp, true>
    : public details::Lz77DummyAuxEncoderBase<Tp> {
    using details::Lz77DummyAuxEncoderBase<Tp>::Lz77DummyAuxEncoderBase;

    using asymetrical = void;
    using token_type = Tp;
};

template <typename Tp, bool IsAsymetrical>
struct Lz77DummyAuxDecoder;

template <typename Tp>
struct Lz77DummyAuxDecoder<Tp, false>
    : public details::Lz77DummyAuxDecoderBase<Tp> {
    using details::Lz77DummyAuxDecoderBase<Tp>::Lz77DummyAuxDecoderBase;

    using token_type = Tp;
};

template <typename Tp>
struct Lz77DummyAuxDecoder<Tp, true>
    : public details::Lz77DummyAuxDecoderBase<Tp> {
    using details::Lz77DummyAuxDecoderBase<Tp>::Lz77DummyAuxDecoderBase;

    using asymetrical = void;
    using token_type = Tp;
};
