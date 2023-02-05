
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
                cmake_policy(SET CMP0135 NEW)
endif()

if (NOT EMSCRIPTEN)
    add_subdirectory(matrix-screen)
    add_subdirectory(lsp-client)
else()
    include(FetchContent)
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
    FetchContent_MakeAvailable(json)
endif()

add_subdirectory(json.h)

find_package (Threads)
