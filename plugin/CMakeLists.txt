

add_library(
    medit_plugins
    STATIC
    clang/clangannotation.cpp
    clang/clangcompletion.cpp
    clang/clangmodel.cpp
    clang/clangnavigation.cpp
    clang/clanghighlight.cpp
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
