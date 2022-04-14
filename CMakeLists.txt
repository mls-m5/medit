
cmake_minimum_required(VERSION 3.23)

project(medit)

add_subdirectory(lib/json.h)
add_subdirectory(lib/matrix-screen)

add_library(
    medit_core
    src/files/config.cpp
    src/files/popenstream.cpp
    src/files/file.cpp
    src/files/project.cpp
    src/files/fontlocator.cpp
    src/keys/keymap.cpp
    src/syntax/basichighligting.cpp
    src/syntax/palette.cpp
    src/core/os.cpp
    src/core/registerdefaultplugins.cpp
    src/core/jsjobqueue.cpp
    src/core/timer.cpp
    src/core/registers.cpp
    src/core/jstimer.cpp
    src/core/plugins.cpp
    src/text/fstring.cpp
    src/text/cursorrangeops.cpp
    src/text/cursorops.cpp
    src/text/cursor.cpp
    src/text/buffer.cpp
    src/text/history.cpp
    src/views/messagebox.cpp
    src/views/editor.cpp
    src/views/locator.cpp
    src/views/mainwindow.cpp
    src/views/inputbox.cpp
    src/views/scrollview.cpp
    src/views/completeview.cpp
    src/views/listview.cpp
    src/views/bufferview.cpp
    src/modes/normalmode.cpp
    src/modes/insertmode.cpp
    src/modes/parentmode.cpp
    src/modes/visualmode.cpp
    src/modes/mode.cpp
    src/script/standardcommands.cpp
    src/script/command.cpp
    src/script/parser.cpp
    src/script/togglecomments.cpp
    src/navigation/headernavigation.cpp
    src/completion/autocomplete.cpp
    src/screen/draw.cpp
    src/screen/ncursesscreen.cpp
    src/screen/linuxterminalscreen.cpp
    src/screen/htmlscreen.cpp
    src/screen/bufferedscreen.cpp
    src/screen/guiscreen.cpp

    plugin/clang/clangannotation.cpp
    plugin/clang/clangcompletion.cpp
    plugin/clang/clangmodel.cpp
    plugin/clang/clangnavigation.cpp
    plugin/clang/clanghighlight.cpp
    plugin/jsonformat.cpp
    plugin/build.cpp
    plugin/windowcommands.cpp
    plugin/clangformat.cpp
    )

find_package (Threads)
target_link_libraries (
    medit_core
    PRIVATE ${CMAKE_THREAD_LIBS_INIT}
    )

find_package(LLVM REQUIRED CONFIG)

target_include_directories(
    medit_core
    PUBLIC
    include
    src
    PRIVATE
    ./
    plugin/include
    plugin
    plugin/clang/include
    /usr/lib/llvm-9/include
    )


target_link_libraries(
    medit_core
    PRIVATE
    json
    matrix-screen
    ncurses
    clang-9
    )

target_compile_features(
    medit_core
    PUBLIC
    cxx_std_17
    )

add_executable(
    medit
    src/main.cpp
    )

target_link_libraries(
    medit
    PRIVATE
    medit_core
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
    data/UbuntuMono-Regular.ttf
    DESTINATION
    ${CMAKE_BINARY_DIR}/data
    )

