#include "initlua.h"
#include "text/cursor.h"
#include "text/cursorops.h"
#include "views/editor.h"
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


)_";

}
void initLua(sol::state &lua) {
    lua.new_usertype<Cursor>("Cursor");

    lua.new_usertype<Editor>("Editor");
    lua["Editor"]["get_cursor"] = [](Editor &e) {
        return e.cursor(); //
    };
    lua["Editor"]["set_cursor"] = [](Editor &e, Cursor &c) {
        return e.cursor(c);
    };

    lua["Cursor"]["fix"] = [](Cursor &self) { return fix(self); };

    lua["print"] = [](std::string str) {
        std::cout << str << std::endl; //
    };

    lua.load(standardLuaCommands)();
}
