#pragma once

#include <koda/coders/coder.hpp>
#include <koda/coders/direct_decoder.hpp>
#include <koda/coders/lzss/lzss_intermediate_token.hpp>
#include <koda/collections/fuzed_dictionary_and_buffer.hpp>
#include <koda/collections/search_binary_tree.hpp>
#include <koda/utils/concepts.hpp>

namespace koda {

template <std::integral Token = uint8_t,
          SizeAwareEncoder<LzssIntermediateToken<Token>> AuxiliaryEncoder =
              DirectDecoder<LzssIntermediateToken<Token>>,
          typename Allocator = std::allocator<Token>>
    requires(sizeof(Token) <= sizeof(LzssIntermediateToken<Token>))
class LzssDecoder : public DecoderInterface < Token,
                    LzssDecoder<Token, AuxiliaryEncoder, Allocator> {
   public:
   private:
};

}  // namespace koda
