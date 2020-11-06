
#include "standardcommands.h"
#include "main.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "plugin/build.h"
#include "plugin/clangformat.h"
#include "plugin/windowcommands.h"
#include "script/parser.h"
#include "text/cursorops.h"
#include "views/editor.h"
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
            e.cursor(erase(e.cursor()));
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
        "editor.format",
        [](IEnvironment &env) { clangFormat(env); },
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
    {"quit", [](auto &) { quitMedit(); }},
};
}

void addStandardCommands(IEnvironment &env) {
    for (auto &pair : editorCommands) {
        env.addCommand(pair.first, pair.second);
    }
}
