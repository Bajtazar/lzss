#pragma once

#include <algorithm>
#include <concepts>

namespace koda::tests {

template <typename Tp>
constexpr bool compare(Tp&& t1, Tp&& t2) noexcept
    requires requires(Tp t1) {
        { t1 == t2 } -> std::same_as<bool>;
    }
{
    return t1 == t2;
}

template <std::input_iterator LeftIter, std::sentinel_for<LeftIter> LeftSent,
          std::input_iterator RightIter, std::sentinel_for<RightIter> RightSent>
constexpr bool compare(LeftIter leftIter, LeftSent leftSent,
                       RightIter rightIter, RightSent rightSent) noexcept {
    return std::ranges::equal(leftIter, leftSent, rightIter, rightSent);
}

template <std::ranges::input_range Tp, std::ranges::input_range Up>
constexpr bool compare(Tp&& t1, Up&& t2) noexcept {
    return std::ranges::equal(std::forward<Tp>(t1), std::forward<Up>(t2));
}

}  // namespace koda::tests

#define BeginConstexprTest(TestName)    \
    constexpr uint64_t TestName(void) { \
        uint64_t ge_test_assertions = 0;

#define ConstexprAssertTrue(assertion)         \
    if (!ge_test_assertions && !(assertion)) { \
        ge_test_assertions = __LINE__;         \
    }

#define ConstexprAssertFalse(assertion)       \
    if (!ge_test_assertions && (assertion)) { \
        ge_test_assertions = __LINE__;        \
    }

#define ConstexprEqual(left, right)                                      \
    if (!ge_test_assertions && !mpgl::tests::compare((left), (right))) { \
        ge_test_assertions = __LINE__;                                   \
    }

#define ConstexprEqualIter(leftIter, leftSent, rightIter, rightSent) \
    if (!ge_test_assertions &&                                       \
        !mpgl::tests::compare((leftIter), (leftSent), (rightIter),   \
                              (rightSent))) {                        \
        ge_test_assertions = __LINE__;                               \
    }

#define ConstexprOnThrow(assertion, exception) \
    try {                                      \
        (assertion);                           \
        if (!ge_test_assertions) {             \
            ge_test_assertions = __LINE__;     \
        }                                      \
    } catch (exception const&) {               \
    }

#define EndConstexprTest(TestName) \
    return ge_test_assertions;     \
    }                              \
    static_assert(TestName() == 0);
