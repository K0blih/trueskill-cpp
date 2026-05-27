cmake_minimum_required(VERSION 3.16)

project(trueskill_cpp VERSION 0.1.0 LANGUAGES CXX)

add_library(
    trueskill
    src/detail/factor_graph.cpp
    src/detail/math.cpp
    src/trueskill.cpp
)

add_library(trueskill::trueskill ALIAS trueskill)

target_compile_features(trueskill PUBLIC cxx_std_20)
target_include_directories(
    trueskill
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

option(TRUESKILL_BUILD_TESTS "Build TrueSkill tests" ON)
option(TRUESKILL_BUILD_CLI "Build TrueSkill CLI" ON)

if(TRUESKILL_BUILD_CLI)
    add_executable(trueskill_cli src/cli/main.cpp)
    target_link_libraries(trueskill_cli PRIVATE trueskill::trueskill)
endif()

if(TRUESKILL_BUILD_TESTS)
    enable_testing()
    add_executable(trueskill_tests tests/trueskill_tests.cpp)
    target_link_libraries(trueskill_tests PRIVATE trueskill::trueskill)
    add_test(NAME trueskill_tests COMMAND trueskill_tests)
    if(TRUESKILL_BUILD_CLI)
        add_test(
            NAME trueskill_cli_quality_smoke
            COMMAND trueskill_cli quality "{\"rating_groups\":[[{\"mu\":25,\"sigma\":8.333333333333}],[{\"mu\":25,\"sigma\":8.333333333333}]]}"
        )
    endif()
endif()
