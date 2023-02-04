cmake_minimum_required(VERSION 3.23)
project(vb6-script-2022)

enable_testing()

function(test_disabled testname)
    add_executable(${testname}_test ${testname}_test.cpp)

    target_include_directories(${testname}_test PRIVATE tests)

    target_compile_features(
        ${testname}_test
        PRIVATE
        cxx_std_17
        )

    target_link_libraries(
        ${testname}_test
        PUBLIC
        medit_common
        lib_medit
        medit_plugins
        )

    target_include_directories(
        ${testname}_test
        PUBLIC
        .
        ../src
        )

endfunction()

function(test testname)
    test_disabled(${testname})

    add_test(NAME ${testname}_test
        COMMAND ${testname}_test
        )
endfunction()

file(
    COPY res
    DESTINATION test
    )

test(basichighlighting)
test(buffer)
test(bufferkeymap)
test(cursorops)
test(cursorrangeops)
test(fstring)
test(history)
test(keysettings)
test(mode)
test(palette)
test(project)
test(utf8char)
test(wordsiterator)

if (ENABLE_LEGACY_CLANG_PLUGIN)
    test(clang_autocomplete)
    test(clanghighlighting)
endif()
