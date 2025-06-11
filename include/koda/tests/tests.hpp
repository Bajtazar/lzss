#pragma once

#include <algorithm>
#include <concepts>

namespace koda::tests {

template <typename Tp, typename Up>
    requires std::equality_comparable_with<Tp, Up>
constexpr bool compare(Tp&& t1, Up&& t2) noexcept {
    return t1 == t2;
}

template <std::input_iterator LeftIter, std::sentinel_for<LeftIter> LeftSent,
          std::input_iterator RightIter, std::sentinel_for<RightIter> RightSent>
constexpr bool compare(LeftIter leftIter, LeftSent leftSent,
                       RightIter rightIter, RightSent rightSent) noexcept {
    return std::ranges::equal(leftIter, leftSent, rightIter, rightSent);
}

template <std::ranges::input_range Tp, std::ranges::input_range Up>
    requires(!std::equality_comparable_with<Tp, Up>)
constexpr bool compare(Tp&& t1, Up&& t2) noexcept {
    return std::ranges::equal(std::forward<Tp>(t1), std::forward<Up>(t2));
}

}  // namespace koda::tests

// clang-format off
#define BeginConstexprTest(TestCase, TestName) \
    static_assert([]() static -> uint64_t { \
        uint64_t ge_test_assertions = 0;
// clang-format on

#define ConstexprAssertTrue(assertion)         \
    if (!ge_test_assertions && !(assertion)) { \
        ge_test_assertions = __LINE__;         \
    }

#define ConstexprAssertFalse(assertion)       \
    if (!ge_test_assertions && (assertion)) { \
        ge_test_assertions = __LINE__;        \
    }

#define ConstexprAssertEqual(left, right)                                \
    if (!ge_test_assertions && !koda::tests::compare((left), (right))) { \
        ge_test_assertions = __LINE__;                                   \
    }

#define ConstexprAssertEqualIter(leftIter, leftSent, rightIter, rightSent) \
    if (!ge_test_assertions &&                                             \
        !koda::tests::compare((leftIter), (leftSent), (rightIter),         \
                              (rightSent))) {                              \
        ge_test_assertions = __LINE__;                                     \
    }

#define ConstexprAssertNotEqual(left, right)                            \
    if (!ge_test_assertions && koda::tests::compare((left), (right))) { \
        ge_test_assertions = __LINE__;                                  \
    }

#define ConstexprAssertNotEqualIter(leftIter, leftSent, rightIter, rightSent) \
    if (!ge_test_assertions &&                                                \
        koda::tests::compare((leftIter), (leftSent), (rightIter),             \
                             (rightSent))) {                                  \
        ge_test_assertions = __LINE__;                                        \
    }

#define ConstexprAssertOnThrow(assertion, exception) \
    try {                                            \
        (assertion);                                 \
        if (!ge_test_assertions) {                   \
            ge_test_assertions = __LINE__;           \
        }                                            \
    } catch (exception const&) {                     \
    }

#define EndConstexprTest       \
    return ge_test_assertions; \
    }() == 0);
