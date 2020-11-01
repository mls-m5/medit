
#include "matscript.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "text/cursorops.h"
#include <functional>
#include <map>
#include <sstream>

namespace {
std::map<std::string_view, std::function<void(IEnvironment &)>> editorCommands =
    {
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
                e.cursor().y() -= 1;
            },
        },
        {
            "editor.down",
            [](IEnvironment &env) {
                auto &e = env.editor();
                e.cursor().y() += 1;
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
            "editor.insertmode",
            [](IEnvironment &env) { env.editor().mode(createInsertMode()); },
        },
        {
            "editor.normalmode",
            [](IEnvironment &env) { env.editor().mode(createNormalMode()); },
        },
        {"quit", [](auto &) { exit(0); }},
};
}

void run(std::string command, IEnvironment &env) {
    auto ss = std::istringstream{std::move(command)};

    for (std::string line; getline(ss, line);) {
        auto f = editorCommands.find(line);
        if (f != editorCommands.end()) {
            (f->second)(env);
        }
    }
}
