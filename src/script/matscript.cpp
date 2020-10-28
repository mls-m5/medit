
#include "matscript.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include <functional>
#include <map>
#include <sstream>

namespace {
std::map<std::string_view, std::function<void(IEnvironment &)>> editorCommands =
    {
        {
            "editor.previous",
            [](IEnvironment &env) {
                auto &e = env.editor();
                e.cursor(e.buffer().prev(e.cursor()));
            },
        },
        {
            "editor.next",
            [](IEnvironment &env) {
                auto &e = env.editor();
                e.cursor(e.buffer().next(e.cursor()));
            },
        },
        {
            "editor.up",
            [](IEnvironment &env) {
                auto &e = env.editor();
                if (e.cursor().y == 0) {
                    return;
                }
                e.cursor().y -= 1;
            },
        },
        {
            "editor.down",
            [](IEnvironment &env) {
                auto &e = env.editor();
                e.cursor().y += 1;
            },
        },
        {
            "editor.insert",
            [](IEnvironment &env) {
                auto &e = env.editor();
                auto event = env.key();
                e.cursor(e.buffer().insert(event.symbol, e.cursor()));
            },
        },
        {
            "editor.erase",
            [](IEnvironment &env) {
                auto &e = env.editor();
                e.cursor(e.buffer().erase(e.cursor()));
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
