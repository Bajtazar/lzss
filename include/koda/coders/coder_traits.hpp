#pragma once

#include <koda/coders/coder.hpp>

namespace koda {

template <typename CoderTp>
    requires(Encoder<CoderTp, typename CoderTp::token_type> ||
             Decoder<CoderTp, typename CoderTp::token_type>)
struct CoderTraits {
    using token_type = typename CoderTp::token_type;

    static constexpr bool IsSizeAware =
        requires(CoderTp coder, token_type token) {
            { coder.TokenBitSize(token) } -> std::same_as<float>;
        };

    static constexpr bool IsAsymetric =
        (requires { typename CoderTp::asymetrical; }) || (requires {
            {
                std::integral_constant<bool, CoderTp::IsAsymetric>{}
            } -> std::same_as<std::true_type>;
        });

    static constexpr bool IsSymetric = !IsAsymetric;
};

}  // namespace koda
