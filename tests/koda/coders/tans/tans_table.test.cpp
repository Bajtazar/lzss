#include <koda/coders/tans/tans_table.hpp>
#include <koda/tests/tests.hpp>

#include <gtest/gtest.h>

#include <print>

namespace koda {

BeginConstexprTest(TansTableTest, UniformDistribution) {
    koda::Map<char, size_t> counter = {{{'a', 2},
                                        {'b', 2},
                                        {'c', 2},
                                        {'d', 2},
                                        {'e', 2},
                                        {'f', 2},
                                        {'g', 2},
                                        {'h', 2}}};

    const std::vector<char> kExpectedStateTable = {'a', 'a', 'b', 'b', 'c', 'c',
                                                   'd', 'd', 'e', 'e', 'f', 'f',
                                                   'g', 'g', 'h', 'h'};

    koda::Map<char, size_t> kExpectedStatesPerToken = counter;
    const size_t kNumberOfStates = 16;

    koda::TansInitTable table{counter};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;

BeginConstexprTest(TansTableTest, GeometricDistribution) {
    koda::Map<char, size_t> counter = {{{'a', 2}, {'b', 4}, {'c', 8}}};

    const std::vector<char> kExpectedStateTable = {
        'a', 'a', 'b', 'b', 'b', 'b', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c'};

    koda::Map<char, size_t> kExpectedStatesPerToken = counter;
    const size_t kNumberOfStates = 14;

    koda::TansInitTable table{counter};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;

BeginConstexprTest(TansTableTest, GeometricDistributionShifted) {
    koda::Map<char, size_t> counter = {{{'a', 2}, {'b', 4}, {'c', 8}}};

    const std::vector<char> kExpectedStateTable = {
        'c', 'c', 'c', 'c', 'c', 'a', 'a', 'b', 'b', 'b', 'b', 'c', 'c', 'c'};

    koda::Map<char, size_t> kExpectedStatesPerToken = counter;
    const size_t kNumberOfStates = 14;

    koda::TansInitTable table{counter, 5};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;

}  // namespace koda
