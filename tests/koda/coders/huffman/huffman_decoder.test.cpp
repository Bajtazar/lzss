#include <koda/coders/huffman/huffman_decoder.hpp>
#include <koda/tests/tests.hpp>
#include <koda/utils/back_inserter_iterator.hpp>
#include <koda/utils/bit_iterator.hpp>

#include "common.hpp"

static_assert(koda::Decoder<koda::HuffmanDecoder<uint8_t>, uint8_t>);
