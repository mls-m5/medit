
cmake_minimum_required(VERSION 3.23)

enable_testing()

project(medit)

add_subdirectory(lib)
add_subdirectory(src)


if(CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(
        -pedantic
        -Wall
        -Wextra
        -Woverloaded-virtual
        -Winit-self
        -Wunreachable-code
        -Wno-unused-parameter
        -Wno-variadic-macros
        )

endif(CMAKE_COMPILER_IS_GNUCXX)

add_library(
    medit_common
    INTERFACE
    )

target_include_directories(
    medit_common
    INTERFACE
    include/
    ${CMAKE_SOURCE_DIR}/src/
    ./
    src/plugin/include
    src/plugin
    src/plugin/clang/include
    )

target_link_libraries(
    medit_common
    INTERFACE
    json
    ${CMAKE_THREAD_LIBS_INIT}
    nlohmann_json::nlohmann_json
    )

target_compile_features(
    medit_common
    INTERFACE
    cxx_std_20
    )

add_subdirectory(test)

# === medit ===================================================

add_executable(
    medit
    src/main.cpp
    src/registerdefaultplugins.cpp
    )

target_link_libraries(
    medit
    PRIVATE
    lib_medit
    medit_common
    medit_plugins
    )

file(
    COPY
    README.md
    DESTINATION
    ${CMAKE_BINARY_DIR}
    )

file(
    COPY
    data/oblivion.json
    data/oblivion-bg-black.json
    data/init.lua
    data/mode.lua
    data/UbuntuMono-Regular.ttf
    DESTINATION
    ${CMAKE_BINARY_DIR}/data
    )

if (EMSCRIPTEN)
    file(
        COPY
        index.html
        DESTINATION
        .
        )

    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -gsource-map ")
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} --source-map-base http://localhost:8000/")
endif()

# === code playback ===================================================

add_executable(
    code_playback
    src/codeplayback/codeplayback.cpp
    )

target_link_libraries(
    code_playback
    PRIVATE
    medit_common
    lib_medit
    medit_plugins ## Should not be required.. Well well
    sdlpp
    SDL2_image
    )

