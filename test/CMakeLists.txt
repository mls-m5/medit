cmake_minimum_required(VERSION 3.23)
project(vb6-script-2022)

enable_testing()

add_executable(
    medit_tests
    tests.cpp
#    basichighlighting_test.cpp
#    buffer_test.cpp
#    bufferkeymap_test.cpp
#    cursorops_test.cpp
#    cursorrangeops_test.cpp
#    fstring_test.cpp
#    history_test.cpp
#    keysettings_test.cpp
#    mode_test.cpp
#    palette_test.cpp
#    project_test.cpp
#    utf8char_test.cpp
#    wordsiterator_test.cpp
#    serializescreen_test.cpp
#    archive_test.cpp
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

#test_disabled(${testname})

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
