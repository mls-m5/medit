
add_subdirectory(plugin)

add_library(
    medit_core
    STATIC
    files/config.cpp
    files/popenstream.cpp
    files/file.cpp
    files/project.cpp
    files/fontlocator.cpp
    keys/keymap.cpp
    syntax/basichighligting.cpp
    syntax/palette.cpp
    core/os.cpp
    core/jsjobqueue.cpp
    core/timer.cpp
    core/registers.cpp
    core/jstimer.cpp
    core/plugins.cpp
    text/fstring.cpp
    text/cursorrangeops.cpp
    text/cursorops.cpp
    text/cursor.cpp
    text/rawbuffer.cpp
    text/buffer.cpp
    text/history.cpp
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
    script/standardcommands.cpp
    script/command.cpp
    script/parser.cpp
    script/togglecomments.cpp
    navigation/headernavigation.cpp
    completion/autocomplete.cpp
    screen/draw.cpp
    screen/ncursesscreen.cpp
    screen/linuxterminalscreen.cpp
    screen/htmlscreen.cpp
    screen/bufferedscreen.cpp
    screen/guiscreen.cpp
    )

find_package (Threads)
target_link_libraries (
    medit_core
    PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
    )

#find_package(LLVM REQUIRED CONFIG)

target_include_directories(
    medit_core
    PUBLIC
    include
    src
    )


target_link_libraries(
    medit_core
    PRIVATE
    medit_common
    matrix-screen
    )

if (NOT EMSCRIPTEN)
    target_link_libraries(
        medit_core
        PRIVATE
        ncurses
    )

endif()
