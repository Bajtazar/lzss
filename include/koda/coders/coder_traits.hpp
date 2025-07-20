#pragma once

#include <koda/coders/coder.hpp>

namespace koda {

template <typename CoderTp>
    requires(Encoder<CoderTp, typename CoderTp::token_type> ||
             Decoder<CoderTp, typename CoderTp::token_type>)
struct CoderTraits {
    using token_type = typename CoderTp::token_type;

    inline constexpr bool IsSizeAware =
        requires(CoderTp coder, token_type token) {
            { coder.TokenBitSize(token) } -> std::same_as<float>;
        };

    inline constexpr bool IsAsymetrical =
        requires { typename CoderTp::asymetrical; };
};

}  // namespace koda
