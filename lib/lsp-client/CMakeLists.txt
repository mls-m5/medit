
cmake_minimum_required(VERSION 3.23)
project(clangd-client LANGUAGES CXX)

add_subdirectory(lib)
add_subdirectory(tests)

add_library(
    lsp-client
    STATIC
    lsp.cpp
    )


target_compile_features(
    lsp-client
    PRIVATE
    cxx_std_17
    )

target_link_libraries(
    lsp-client
    PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    nlohmann_json::nlohmann_json
    )


target_include_directories(
    lsp-client
    PUBLIC
    include
    )


file(COPY src DESTINATION .)
file(COPY tests DESTINATION .)
