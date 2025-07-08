#include <koda/coders/lz77/lz77_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

static_assert(koda::Encoder<koda::Lz77Encoder<uint8_t>, uint8_t>);

namespace {

template <typename Tp>
struct Lz77DummyAuxEncoder
    : public koda::EncoderInterface<Tp, Lz77DummyAuxEncoder<Tp>> {
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

}  // namespace
