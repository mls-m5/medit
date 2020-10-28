
#include "matscript.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include <functional>
#include <map>

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
            "editor.insertmode",
            [](IEnvironment &env) {
                env.editor().mode(std::make_unique<InsertMode>(env));
            },
        },
        {
            "editor.normalmode",
            [](IEnvironment &env) {
                env.editor().mode(std::make_unique<NormalMode>(env));
            },
        },
};
}

void run(std::string_view command, IEnvironment &env) {
    auto f = editorCommands.find(command);
    if (f != editorCommands.end()) {
        (f->second)(env);
    }
}
