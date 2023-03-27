
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
                cmake_policy(SET CMP0135 NEW)
endif()

if (NOT EMSCRIPTEN)
    add_subdirectory(matrix-screen)
    add_subdirectory(lsp-client)
endif()

# --- nlohmann json ---------------------

include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

# --- json.h ----------------------------

add_subdirectory(json.h)

# --- Threads ---------------------------

find_package (Threads)

# --- lua -------------------------------

#file(
#    GLOB
#    LUA_FILES
#    "lua/*.c"
#    )

#add_library(
#    lua
#    ${LUA_FILES}
#    )

#target_include_directories(
#    lua
#    PUBLIC
#    lua
#    )

# --- sol2 ---------------------------

#add_subdirectory(sol2)


