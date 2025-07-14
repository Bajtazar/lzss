#include <koda/coders/tans/tans_table.hpp>
#include <koda/tests/tests.hpp>

BeginConstexprTest(TansTableTest, UniformDistribution) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2},
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

    koda::Map<char, size_t> kExpectedStatesPerToken = kCounter;
    const size_t kNumberOfStates = 16;

    koda::TansInitTable table{kCounter};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;

BeginConstexprTest(TansTableTest, GeometricDistribution) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2}, {'b', 4}, {'c', 8}}};

    const std::vector<char> kExpectedStateTable = {
        'a', 'a', 'b', 'b', 'b', 'b', 'c', 'c', 'c', 'c', 'c', 'c', 'c', 'c'};

    const koda::Map<char, size_t> kExpectedStatesPerToken = kCounter;
    const size_t kNumberOfStates = 14;

    koda::TansInitTable table{kCounter};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;

BeginConstexprTest(TansTableTest, GeometricDistributionShifted) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2}, {'b', 4}, {'c', 8}}};

    const std::vector<char> kExpectedStateTable = {
        'c', 'c', 'c', 'c', 'c', 'a', 'a', 'b', 'b', 'b', 'b', 'c', 'c', 'c'};

    const koda::Map<char, size_t> kExpectedStatesPerToken = kCounter;
    const size_t kNumberOfStates = 14;

    koda::TansInitTable table{kCounter, 5};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;

BeginConstexprTest(TansTableTest, UniformDistributionStrided) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2},
                                               {'b', 2},
                                               {'c', 2},
                                               {'d', 2},
                                               {'e', 2},
                                               {'f', 2},
                                               {'g', 2},
                                               {'h', 2}}};

    const std::vector<char> kExpectedStateTable = {'b', 'e', 'a', 'd', 'h', 'c',
                                                   'g', 'b', 'f', 'a', 'e', 'h',
                                                   'd', 'g', 'c', 'f'};

    const koda::Map<char, size_t> kExpectedStatesPerToken = kCounter;
    const size_t kNumberOfStates = 16;

    koda::TansInitTable table{kCounter, 2, 7};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;

BeginConstexprTest(TansTableTest, UniformDistributionStridedAndScaled) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2},
                                               {'b', 2},
                                               {'c', 2},
                                               {'d', 2},
                                               {'e', 2},
                                               {'f', 2},
                                               {'g', 2},
                                               {'h', 2}}};

    const std::vector<char> kExpectedStateTable = {
        'g', 'h', 'a', 'b', 'd', 'e', 'f', 'g', 'h', 'b', 'c', 'd', 'e'};

    const koda::Map<char, size_t> kExpectedStatesPerToken = {{{'a', 1},
                                                              {'b', 2},
                                                              {'c', 1},
                                                              {'d', 2},
                                                              {'e', 2},
                                                              {'f', 1},
                                                              {'g', 2},
                                                              {'h', 2}}};
    const size_t kNumberOfStates = 13;

    koda::TansInitTable table{kCounter, 2, 7, 13};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;
