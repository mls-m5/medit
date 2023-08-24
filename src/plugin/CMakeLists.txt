
enable_testing()

add_library(
    medit_plugins
    STATIC
    plugin.cpp
    )


target_link_libraries(
    medit_plugins
    PUBLIC
    medit_common
    lib_medit
    )

if (NOT EMSCRIPTEN)

    target_link_libraries(
        medit_plugins
        PUBLIC
        lsp-client
        )

    target_sources(
        medit_plugins
        PRIVATE
        lsp/lspplugin.cpp
        )

endif()

if (ENABLE_LEGACY_CLANG_PLUGIN)

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
        lib_medit
        clang-9
        )

    target_link_libraries(
        medit_plugins
        PUBLIC
        medit_clang_plugins
        )

endif()
