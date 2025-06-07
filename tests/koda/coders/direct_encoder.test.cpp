#include <koda/coders/direct_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <gtest/gtest.h>

#include <iterator>
#include <vector>

namespace {

class SimpleInputBinaryRange {
   public:
    using BitIter =
        koda::LittleEndianInputBitIter<typename std::vector<uint8_t>::iterator>;

    constexpr explicit SimpleInputBinaryRange(std::vector<uint8_t> vector)
        : vector_{std::move(vector)}, input_iter_{vector_.begin()} {}

    constexpr BitIter& begin() const noexcept { return input_iter_; }

    constexpr BitIter end() const noexcept { return BitIter{}; }

   private:
    std::vector<uint8_t> vector_;
    mutable BitIter input_iter_;
};

class SimpleOutputBinaryRange {
   public:
    using BackInserter = koda::BackInserterIterator<std::vector<uint8_t>>;
    using BitIter = koda::LittleEndianOutputBitIter<BackInserter>;

    constexpr explicit SimpleOutputBinaryRange()
        : output_iter_{BackInserter{vector}} {}

    constexpr BitIter& begin() noexcept { return output_iter_; }

    constexpr std::default_sentinel_t end() noexcept {
        return std::default_sentinel;
    }

    std::vector<uint8_t> vector;

   private:
    BitIter output_iter_;
};

}  // namespace

BeginConstexprTest(DirectEncoderTest, EncodeBytes) {
    const std::vector<uint8_t> expected{{0x43, 0x74, 0x35, 0x33}};
    SimpleOutputBinaryRange range;
    koda::DirectEncoder<uint8_t> encoder;

    encoder.Encode(expected, range);

    ConstexprAssertEqual(expected, range.vector);
}
EndConstexprTest(DirectEncoderTest, EncodeBytes);
