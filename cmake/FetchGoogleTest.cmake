include(FetchContent)

macro(FetchGoogleTest)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    message("-- Fetching googletest")

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )

    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(GTEST_LIBRARIES GTest::gtest_main)
endif()

endmacro()
