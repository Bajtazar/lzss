cmake_minimum_required(VERSION 3.17)

project(lzss VERSION 1)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(LZSS_FLAGS "-fconstexpr-ops-limit=1000000000 -fconcepts-diagnostics-depth=6")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${LZSS_FLAGS}")

option(LZSS_TESTS "enables lzss tests" ON)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/src)

if(LZSS_TESTS)
    include(FetchGoogleTest)

    enable_testing()

    FetchGoogleTest()

    file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/*.cpp)

    add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

    target_include_directories(${PROJECT_NAME}_tests
        PUBLIC ${PROJECT_SOURCE_DIR}/tests)

    target_link_directories(${PROJECT_NAME}_tests
        PUBLIC ${PROJECT_SOURCE_DIR}/tests)

    target_link_libraries(${PROJECT_NAME}_tests ${PROJECT_NAME} ${GTEST_LIBRARIES})

    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_tests)
endif()

