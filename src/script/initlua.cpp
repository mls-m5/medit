#include "initlua.h"
#include "files/config.h"
#include "sol/error.hpp"
#include "sol/forward.hpp"
#include "text/cursor.h"
#include "text/cursorops.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <memory>
#include <stdexcept>
#include <string>

namespace {

constexpr auto standardLuaCommands = R"_(


)_";

}

void initLua(sol::state &lua, MainWindow &mainWindow) {
    lua.open_libraries(sol::lib::base);

    lua.new_usertype<Buffer>("Buffer", "is_single_line", &Buffer::isSingleLine);

    lua.new_usertype<Cursor>(
        "Cursor",
        "get_buffer",
        [](Cursor &self) { return &self.buffer(); },
        "get_x",
        [](Cursor &self) { return self.x(); },
        "get_y",
        [](Cursor &self) { return self.y(); });

    lua.new_usertype<Editor>(
        "Editor", "get_cursor", [](Editor &self) { return self.cursor(); });

    lua.new_usertype<MainWindow>("Window", "get_editor", [](MainWindow &self) {
        return &self.currentEditor();
    });

    lua["window"] = &mainWindow;

    lua["print"] = [](std::string str) { std::cout << str << std::endl; };

    lua["bind"] = [](std::string key, sol::function f) {};

    lua["goto_definition"] = []() {
        std::cout << "goto_definition called" << std::endl;
    };

    try {
        auto initFile = findConfig("data/init.lua");
        auto res = lua.script_file(initFile);

        if (!res.valid()) {
            std::cerr << sol::error{res}.what() << "\n";
        }
    }
    catch (sol::error &e) {
        std::cerr << e.what() << std::endl;
    }
}
