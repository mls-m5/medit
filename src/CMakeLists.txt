
add_subdirectory(plugin)

# ------------- shared files ----------------------------
add_library(
    medit_core
    STATIC
    core/coreenvironment.cpp
    core/os.cpp
    core/jsjobqueue.cpp
    core/timer.cpp
    core/threadvalidation.cpp
    core/registers.cpp
    core/jstimer.cpp
    core/plugins.cpp
    syntax/palette.cpp
    )

target_link_libraries(
    medit_core
    PUBLIC
    medit_common
    lua
    sol2
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
    screen/serializescreen.cpp
    screen/deserializescreen.cpp
    remote/fifoconnection.cpp
    remote/fifofile.cpp
    remote/tcpconnection.cpp
    remote/tcpserver.cpp
    thinmain.cpp
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
    files/config.cpp
    files/popenstream.cpp
    files/file.cpp
    files/project.cpp
    keys/keymap.cpp
    syntax/basichighligting.cpp
    text/fstring.cpp
    text/cursorrangeops.cpp
    text/cursorops.cpp
    text/cursor.cpp
    text/rawbuffer.cpp
    text/buffer.cpp
    text/history.cpp
    text/diagnostics.cpp
    text/bufferedit.cpp
    views/messagebox.cpp
    views/editor.cpp
    views/locator.cpp
    views/mainwindow.cpp
    views/inputbox.cpp
    views/scrollview.cpp
    views/completeview.cpp
    views/listview.cpp
    views/bufferview.cpp
    modes/normalmode.cpp
    modes/insertmode.cpp
    modes/parentmode.cpp
    modes/visualmode.cpp
    modes/mode.cpp
    script/localenvironment.cpp
    script/standardcommands.cpp
    script/command.cpp
    script/parser.cpp
    script/togglecomments.cpp
    script/luastate.cpp
    script/scope.cpp
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


