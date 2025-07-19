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

    const std::vector<char> kExpectedStateTable = {'a', 'a', 'b', 'b', 'b', 'b',
                                                   'c', 'c', 'c', 'c', 'c', 'c',
                                                   'c', 'c', 'c', 'c'};

    const koda::Map<char, size_t> kExpectedStatesPerToken = {
        {{'a', 2}, {'b', 4}, {'c', 10}}};
    const size_t kNumberOfStates = 16;

    koda::TansInitTable table{kCounter, 0, 1, kNumberOfStates};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;

BeginConstexprTest(TansTableTest, GeometricDistributionShifted) {
    const koda::Map<char, size_t> kCounter = {{{'a', 2}, {'b', 4}, {'c', 8}}};

    const std::vector<char> kExpectedStateTable = {'c', 'c', 'c', 'c', 'c', 'a',
                                                   'a', 'b', 'b', 'b', 'b', 'c',
                                                   'c', 'c', 'c', 'c'};

    const koda::Map<char, size_t> kExpectedStatesPerToken = {
        {{'a', 2}, {'b', 4}, {'c', 10}}};
    const size_t kNumberOfStates = 16;

    koda::TansInitTable table{kCounter, 5, 1, kNumberOfStates};

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
        'e', 'c', 'a', 'f', 'd', 'b', 'h', 'e', 'c', 'a', 'g',
        'd', 'b', 'h', 'f', 'c', 'a', 'g', 'e', 'b', 'h', 'f',
        'd', 'a', 'g', 'e', 'c', 'h', 'f', 'd', 'b', 'g'};

    const koda::Map<char, size_t> kExpectedStatesPerToken = {{{'a', 4},
                                                              {'b', 4},
                                                              {'c', 4},
                                                              {'d', 4},
                                                              {'e', 4},
                                                              {'f', 4},
                                                              {'g', 4},
                                                              {'h', 4}}};
    const size_t kNumberOfStates = 32;

    koda::TansInitTable table{kCounter, 2, 7, kNumberOfStates};

    ConstexprAssertEqual(table.state_table(), kExpectedStateTable);
    ConstexprAssertEqual(table.states_per_token(), kExpectedStatesPerToken);
    ConstexprAssertEqual(table.number_of_states(), kNumberOfStates);
}
EndConstexprTest;
