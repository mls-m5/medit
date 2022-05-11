
cmake_minimum_required(VERSION 3.23)

enable_testing()

project(medit)

add_subdirectory(lib/json.h)
add_subdirectory(lib/matrix-screen)
add_subdirectory(plugin)
add_subdirectory(src)

add_library(
    medit_common
    INTERFACE
    )

target_include_directories(
    medit_common
    INTERFACE
    include/
    src/
    ./
    plugin/include
    plugin
    plugin/clang/include
    /usr/lib/llvm-9/include
    )

target_link_libraries(
    medit_common
    INTERFACE
    json
    )

target_compile_features(
    medit_common
    INTERFACE
    cxx_std_17
    )

# ======================================================

add_executable(
    medit
    src/main.cpp
    src/registerdefaultplugins.cpp
    )

target_link_libraries(
    medit
    PRIVATE
    medit_core
    medit_common
    medit_plugins
    )

if (NOT EMSCRIPTEN)
    target_link_libraries(
        medit
        PRIVATE
        medit_clang_plugins
        )
endif()

file(
    COPY
    README.md
    DESTINATION
    ${CMAKE_BINARY_DIR}
    )

file(
    COPY
    data/oblivion.json
    data/UbuntuMono-Regular.ttf
    DESTINATION
    ${CMAKE_BINARY_DIR}/data
    )

