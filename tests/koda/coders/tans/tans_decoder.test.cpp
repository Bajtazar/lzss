#include <koda/coders/tans/tans_decoder.hpp>
#include <koda/coders/tans/tans_table.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include <cinttypes>

static_assert(koda::Decoder<koda::TansDecoder<uint8_t, size_t>, uint8_t>);
