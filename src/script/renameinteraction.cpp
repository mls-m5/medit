#include "renameinteraction.h"
#include "ienvironment.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <memory>

void renameInteraction(std::shared_ptr<IEnvironment> env) {
    // TODO: Move this to separate file
    auto &e = env->editor();
    auto cursor = e.cursor();
    auto range = CursorRange{::wordBegin(cursor), ::right(::wordEnd(cursor))};
    auto old = content(range);

    auto replace = [](std::string str, std::string_view c, std::string insert) {
        auto position = str.find(c);
        str.replace(position, c.size(), insert);
        return str;
    };

    std::string str = R"(
rename
from: xxx_placeholder
to: yyy_placeholder
location: zzz_placeholder
# Press return to accept, esc to abort
)";

    str = replace(str, "xxx_placeholder", old);
    str = replace(str, "yyy_placeholder", old);
    if (auto f = e.file()) {
        // TODO: Handle for buffers that is not saved
        str = replace(str,
                      "zzz_placeholder",
                      e.file()->path().string() + ":" +
                          std::to_string(cursor.y()) + "," +
                          std::to_string(cursor.x()));
    }

    insert(env->console().buffer().end(), str);
    insert(env->console().buffer().end(),
           "\n#rename symbol is not implemented yet...");
    env->console().cursor(env->console().buffer().end());
    env->showConsole(true);
}
