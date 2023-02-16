#include "initlua.h"
#include "sol/error.hpp"
#include "text/cursor.h"
#include "text/cursorops.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <memory>
#include <stdexcept>
#include <string>

namespace {

constexpr auto standardLuaCommands = R"_(

-- -- possible syntax:
-- function Editor.select_inner_word(self) {
--     cursor = e.cursor()
--    range = createCursorRange(wordBegin(cursor), right(wordEnd(cursor)))
--     e.selection(range)
-- }


-- c = editor:get_cursor()


print("hello")

-- print(Cursor)
  --c = Apa.new{}

print(tostring(apa:get_x()))


editor = window:get_editor()
cursor = editor:get_cursor()

print("cursor x")

print(tostring(cursor:get_x()))

print("hello2")

)_";

}

struct Apa {
    int x = 10;

    int get_x() {
        return x;
    }

    Apa() {
        std::cout << "constructor" << std::endl;
    }

    ~Apa() {
        std::cout << "destructor" << std::endl;
    }
};

void initLua(sol::state &lua, MainWindow &mainWindow) {
    lua.open_libraries(sol::lib::base); // Neaded for usertypes for some reason

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

    {
        // Test
        auto &editor = mainWindow.currentEditor();
        editor.cursor(right(down(editor.cursor())));
    }

    lua.new_usertype<MainWindow>("Window", "get_editor", [](MainWindow &self) {
        return &self.currentEditor();
    });

    lua["window"] = &mainWindow;

    lua.new_usertype<Apa>("Apa", "get_x", &Apa::get_x);

    lua["apa"] = Apa{};

    lua["print"] = [](std::string str) { std::cout << str << std::endl; };

    try {
        lua.script(standardLuaCommands);
    }
    catch (sol::error &e) {
        std::cerr << e.what() << std::endl;
    }
}
