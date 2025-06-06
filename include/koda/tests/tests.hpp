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

#define BeginConstexprTest(TestName) \
    constexpr bool TestName(void) {  \
        bool ge_test_assertions = true;

#define ConstexprAssertTrue(assertion) \
    ge_test_assertions = ge_test_assertions && (assertion);

#define ConstexprAssertFalse(assertion) \
    ge_test_assertions = ge_test_assertions && !(assertion);

#define ConstexprEqual(left, right) \
    ge_test_assertions =            \
        ge_test_assertions && mpgl::tests::compare((left), (right));

#define ConstexprEqualIter(leftIter, leftSent, rightIter, rightSent) \
    ge_test_assertions =                                             \
        ge_test_assertions &&                                        \
        mpgl::tests::compare((leftIter), (leftSent), (rightIter), (rightSent));

#define ConstexprOnThrow(assertion, exception) \
    try {                                      \
        (assertion);                             \
        ge_test_assertions = false;            \
    } catch (exception const&) {               \
    }

#define EndConstexprTest(TestName)       \
    return ge_test_assertions; \
    } \
    static_assert(TestName());

