

if (NOT EMSCRIPTEN)
    add_subdirectory(matrix-screen)
endif()

add_subdirectory(json.h)
add_subdirectory(lsp-client)

find_package (Threads)
