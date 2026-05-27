cmake_minimum_required(VERSION 3.16)

project(trueskill_cpp VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

option(TRUESKILL_BUILD_TESTS "Build TrueSkill tests" ON)
option(TRUESKILL_BUILD_CLI "Build TrueSkill CLI" ON)
option(TRUESKILL_BUILD_HTTP "Build TrueSkill HTTP service" ON)

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

if(TRUESKILL_BUILD_CLI OR TRUESKILL_BUILD_HTTP)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
    )
    FetchContent_MakeAvailable(nlohmann_json)

    add_library(
        trueskill_app
        src/app/commands.cpp
        src/app/json.cpp
    )
    target_compile_features(trueskill_app PUBLIC cxx_std_20)
    target_include_directories(trueskill_app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(trueskill_app PUBLIC trueskill::trueskill nlohmann_json::nlohmann_json)
endif()

if(TRUESKILL_BUILD_CLI)
    add_executable(trueskill_cli src/cli/main.cpp)
    target_include_directories(trueskill_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(trueskill_cli PRIVATE trueskill_app)
endif()

if(TRUESKILL_BUILD_HTTP)
    FetchContent_Declare(
        cpp_httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.26.0
    )
    FetchContent_MakeAvailable(cpp_httplib)

    add_executable(trueskill_http src/http/main.cpp)
    target_include_directories(trueskill_http PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(trueskill_http PRIVATE trueskill_app httplib::httplib)
endif()

if(TRUESKILL_BUILD_TESTS)
    enable_testing()
    add_executable(trueskill_tests tests/trueskill_tests.cpp)
    target_link_libraries(trueskill_tests PRIVATE trueskill::trueskill)
    add_test(NAME trueskill_tests COMMAND trueskill_tests)
    if(TRUESKILL_BUILD_CLI)
        add_test(
            NAME trueskill_cli_smoke
            COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_smoke.sh $<TARGET_FILE:trueskill_cli>
        )
    endif()
    if(TRUESKILL_BUILD_HTTP)
        add_test(
            NAME trueskill_http_smoke
            COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/tests/http_smoke.sh $<TARGET_FILE:trueskill_http> 18080
        )
    endif()
endif()
