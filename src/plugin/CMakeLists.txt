
enable_testing()

add_library(
    medit_plugins
    STATIC
    jsonformat.cpp
    build.cpp
    windowcommands.cpp
    clangformat.cpp
    )


target_link_libraries(
    medit_plugins
    PUBLIC
    medit_common
    medit_core
    )

if (NOT EMSCRIPTEN)

add_library(
    medit_clang_plugins
    STATIC
    clang/clangannotation.cpp
    clang/clangcompletion.cpp
    clang/clangmodel.cpp
    clang/clangnavigation.cpp
    clang/clanghighlight.cpp
    )

target_link_libraries(
    medit_clang_plugins
    PUBLIC
    medit_common
    medit_core
    clang-9
    )

target_link_libraries(
    medit_plugins
    PUBLIC
    medit_clang_plugins
    )

endif()
