
#include "markdowntools.h"
#include "script/ienvironment.h"
#include "text/cursorops.h"
#include "text/cursorrange.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <iostream>
#include <memory>

namespace {

void toggleCheckMark(std::shared_ptr<IEnvironment> env) {
    auto line = ::line(env->editor().cursor());

    Cursor c = line.begin();

    bool found = false;

    for (; c != line.end(); ++c) {
        if (content(c) == '-') {
            found = true;
            break;
        }
    }

    if (!found) {
        return;
    }

    ++c;
    if (content(c) != ' ') {
        return;
    }

    ++c;
    if (content(c) != '[') {
        return;
    }

    ++c;
    auto check = c;

    ++c;

    if (content(c) != ']') {
        return;
    }

    // std::cout << "check value '" << (char)content(check) << "'" << std::endl;

    auto range = CursorRange{check, c};

    if (content(check) == ' ') {
        replace(range, "x");
    }
    else {
        replace(range, " ");
    }
}

} // namespace

void registerMarkdownTools(StandardCommands &sc) {
    sc.addCommand("toggle_checkmark", toggleCheckMark, nullptr);
}
