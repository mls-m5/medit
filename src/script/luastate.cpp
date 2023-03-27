// #include "luastate.h"
// #include "files/config.h"
// #include "keys/event.h"
// #include "meditfwd.h"
// #include "sol/error.hpp"
// #include "sol/forward.hpp"
// #include "text/cursor.h"
// #include "text/cursorops.h"
// #include "views/editor.h"
// #include "views/mainwindow.h"
// #include <filesystem>
// #include <memory>
// #include <stdexcept>
// #include <string>

// void LuaState::init(MainWindow &window) {
//     _lua.open_libraries(sol::lib::base, sol::lib::package);

//    registerTypes();
//    standardFunctions();

//    _lua["window"] = &window;

//    _lua["findScript"] = [this](std::string str) -> std::string {
//        return std::filesystem::relative(findConfig(str),
//                                         std::filesystem::current_path())
//            .replace_extension("");
//    };

//    try {
//        auto initFile = findConfig("data/init.lua");
//        auto res = _lua.script_file(initFile);

//        if (!res.valid()) {
//            std::cerr << sol::error{res}.what() << "\n";
//        }
//    }
//    catch (sol::error &e) {
//        std::cerr << e.what() << std::endl;
//    }
//}

// void LuaState::registerTypes() {

//    _lua.new_usertype<Buffer>(
//        "Buffer", "is_single_line", &Buffer::isSingleLine);

//    _lua.new_usertype<Cursor>(
//        "Cursor",
//        "get_buffer",
//        [](Cursor &self) { return &self.buffer(); },
//        "get_x",
//        [](Cursor &self) { return self.x(); },
//        "get_y",
//        [](Cursor &self) { return self.y(); });

//    _lua.new_usertype<Editor>(
//        "Editor", "get_cursor", [](Editor &self) { return self.cursor(); });

//    _lua.new_usertype<IEnvironment>("Environment");

//    _lua.new_usertype<KeyEvent>(
//        "KeyEvent",
//        "get_symbol",
//        [](KeyEvent &self) { return self.symbol; },
//        "get_modifiers",
//        [](KeyEvent &self) { return self.modifiers; });

//    _lua["translateKeyEvent"] = &KeyEvent::translate;

//    _lua.new_usertype<MainWindow>("Window", "get_editor", [](MainWindow &self)
//    {
//        return &self.currentEditor();
//    });
//}

// void LuaState::standardFunctions() {
//     _lua["print"] = [](std::string str) { std::cout << str << std::endl; };

//    _lua["goto_definition"] = []() {
//        std::cout << "goto_definition called" << std::endl;
//    };
//}
