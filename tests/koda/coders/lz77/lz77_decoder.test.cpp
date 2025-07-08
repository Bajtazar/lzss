#include <koda/coders/lz77/lz77_decoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

static_assert(koda::Decoder<koda::Lz77Decoder<uint8_t>, uint8_t>);

namespace {

template <typename Tp>
class Lz77DummyAuxDecoder
    : public koda::DecoderInterface<Tp, Lz77DummyAuxDecoder<Tp>> {
   public:
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

}
