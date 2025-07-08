#include <koda/coders/lz77/lz77_encoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

static_assert(koda::Encoder<koda::Lz77Encoder<uint8_t>, uint8_t>);
