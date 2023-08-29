


add_subdirectory(plugin)

# ------------- shared files ----------------------------
add_library(
    medit_core
    STATIC
    core/coreenvironment.cpp
    core/files.cpp # Cannot be put in core.cpp for some reason
    core.cpp
    text.cpp
    files/file.cpp
    )

target_link_libraries(
    medit_core
    PUBLIC
    medit_common
    )


# ------------------------- gui screen --------------------

if (NOT EMSCRIPTEN)
add_library(
    medit_sdl_screen
    STATIC
    screen/guiscreen.cpp
    files/fontlocator.cpp
    )


target_link_libraries(
    medit_sdl_screen
    PUBLIC
        medit_common
        medit_core
    PRIVATE
        matrix-screen
    )
endif()


# ---------------------- ncurses screen --------------------

if (NOT EMSCRIPTEN)
    add_library(
        medit_terminal_screen
        STATIC
        screen/ncursesscreen.cpp
        )

    target_link_libraries(
        medit_terminal_screen
        PUBLIC
            medit_common
            medit_core
        PRIVATE
            ncurses
        )
endif()

# --------------------- remote screen ----------------------

add_library(
    medit_remote_screen
    STATIC
    remote_screen.cpp
    )

target_link_libraries(
    medit_remote_screen
    PUBLIC
        medit_common
        medit_core
        nlohmann_json::nlohmann_json
    PRIVATE
    )

# ---------------------- master lib ------------------------
add_library(
    lib_medit
    STATIC
    keys/keymap.cpp
    syntax/basichighligting.cpp
    views.cpp
    modes.cpp
    script.cpp
    navigation/headernavigation.cpp
    completion/autocomplete.cpp
    screen/draw.cpp
    screen/linuxterminalscreen.cpp
    screen/htmlscreen.cpp
    screen/bufferedscreen.cpp
    )

target_link_libraries (
    lib_medit
    PUBLIC
    medit_core
    medit_common
    medit_remote_screen
    )

if (NOT EMSCRIPTEN)
    target_link_libraries (
        lib_medit
        PUBLIC
        medit_sdl_screen
        medit_terminal_screen
        medit_remote_screen
        )
endif()

target_link_libraries(
    lib_medit
    PRIVATE
    medit_common
    )



