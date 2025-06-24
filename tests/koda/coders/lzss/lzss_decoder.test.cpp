#include <koda/coders/lzss/lzss_decoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

namespace {
template <typename Tp>
struct LzssDummyAuxDecoder
    : public koda::DecoderInterface<Tp, LzssDummyAuxDecoder<Tp>> {
    std::vector<Tp> tokens = {};

    constexpr float TokenBitSize(Tp token) const {
        return koda::TokenTraits<Tp>::TokenBitSize(token);
    }

    constexpr auto Initialize(koda::BitInputRange auto&& input) {
        return std::forward<decltype(input)>(input);
    }

    constexpr auto Decode(koda::BitInputRange auto&& input,
                          std::ranges::output_range<Tp> auto&& output) {
        auto res = std::ranges::copy(tokens, output);

        return koda::CoderResult{
            std::move(input), std::ranges::subrange{std::move(res.out),
                                                    std::ranges::end(output)}};
    }
};

}  // namespace
