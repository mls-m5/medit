cmake_minimum_required(VERSION 3.23)
project(vb6-script-2022)

enable_testing()

add_executable(
    medit_tests
    tests.cpp
    mls-unit-test/test_main.cpp
)

target_include_directories(medit_tests PRIVATE tests)

target_compile_features(
    medit_tests
    PRIVATE
    cxx_std_17
    )

target_link_libraries(
    medit_tests
    PUBLIC
    medit_common
    lib_medit
    medit_plugins
    )

target_include_directories(
    medit_tests
    PUBLIC
    .
    ../src
    )

add_test(NAME medit_tests
    COMMAND medit_tests
    )

file(
    COPY res
    DESTINATION test
    )


if (ENABLE_LEGACY_CLANG_PLUGIN)
    test(clang_autocomplete)
    test(clanghighlighting)
endif()
