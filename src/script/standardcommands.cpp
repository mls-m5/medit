
#include "standardcommands.h"
#include "files/project.h"
#include "main.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "modes/visualmode.h"
#include "plugin/build.h"
#include "plugin/clangformat.h"
#include "plugin/windowcommands.h"
#include "script/environment.h"
#include "script/parser.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include "clang/clangnavigation.h" // extract to plugin later
#include <functional>
#include <map>

namespace {
std::map<std::string, std::function<void(IEnvironment &)>> editorCommands = {
    {
        "editor.left",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(left(e.cursor()));
        },
    },
    {
        "editor.right",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(right(e.cursor()));
        },
    },
    {
        "editor.up",
        [](IEnvironment &env) {
            auto &e = env.editor();
            if (e.cursor().y() == 0) {
                return;
            }
            auto cursor = e.cursor();
            cursor.y(cursor.y() - 1);
            e.cursor(cursor);
        },
    },
    {
        "editor.down",
        [](IEnvironment &env) {
            auto &e = env.editor();
            auto cursor = e.cursor();
            cursor.y(e.cursor().y() + 1);
            e.cursor(cursor);
        },
    },
    {
        "editor.word_begin",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(wordBegin(e.cursor()));
        },
    },
    {
        "editor.word_end",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(wordEnd(e.cursor()));
        },
    },
    {
        "editor.delete_line",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(deleteLine(e.cursor()));
        },
    },
    {
        "editor.delete_word",
        [](IEnvironment &env) {
            auto &e = env.editor();
            auto cursor = e.cursor();
            e.cursor(erase({cursor, right(wordEnd(cursor))}));
        },
    },
    {
        "editor.delete_iw",
        [](IEnvironment &env) {
            auto &e = env.editor();
            auto cursor = e.cursor();
            e.cursor(erase({wordBegin(cursor), right(wordEnd(cursor))}));
        },
    },
    {
        "editor.insert",
        [](IEnvironment &env) {
            auto &e = env.editor();
            auto event = env.key();
            e.cursor(insert(event.symbol, e.cursor()));
        },
    },
    {
        "editor.split",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(split(e.cursor()));
        },
    },
    {
        "editor.home",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(home(e.cursor()));
        },
    },
    {
        "editor.end",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(end(e.cursor()));
        },
    },
    {
        "editor.erase",
        [](IEnvironment &env) {
            auto &e = env.editor();
            auto selection = e.selection();
            if (selection.empty()) {
                e.cursor(erase(e.cursor()));
            }
            else {
                e.cursor(erase(selection));
            }
        },
    },
    {
        "editor.join",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(join(e.cursor()));
        },
    },
    {
        "editor.save",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.save();
        },
    },
    {
        "editor.copyindentation",
        [](IEnvironment &env) {
            auto &e = env.editor();
            e.cursor(copyIndentation(e.cursor()));
        },
    },
    {
        "editor.build",
        [](IEnvironment &env) { build(env); },
    },
    {
        "editor.escape",
        [](IEnvironment &env) { escape(env); },
    },
    {
        "editor.insertmode",
        [](IEnvironment &env) { env.editor().mode(createInsertMode()); },
    },
    {
        "editor.normalmode",
        [](IEnvironment &env) { env.editor().mode(createNormalMode()); },
    },
    {
        "editor.visualmode",
        [](IEnvironment &env) { env.editor().mode(createVisualMode()); },
    },
    {
        "editor.goto_definition",
        [](IEnvironment &env) { ClangNavigation::gotoSymbol(env); },
    },
    {
        "editor.switch_header",
        [](IEnvironment &env) {
            auto path = env.project().findSwitchHeader(env.editor().path());
            if (!path.empty()) {
                env.set("path", path.string());
                env.run(parse("editor.open"));
            }
        },
    },
    {"quit", [](auto &) { quitMedit(); }},
};
} // namespace

void addStandardCommands(IEnvironment &env) {
    for (auto &pair : editorCommands) {
        env.addCommand(pair.first, pair.second);
    }
}
