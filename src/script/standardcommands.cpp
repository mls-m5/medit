
#include "standardcommands.h"
#include "core/registers.h"
#include "files/project.h"
#include "main.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "modes/visualmode.h"
#include "plugin/build.h"
#include "plugin/run.h"
#include "script/parser.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "togglecomments.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <functional>
#include <map>

namespace {
using CommandList =
    std::map<std::string, std::function<void(std::shared_ptr<IScope>)>>;

CommandList editorCommands = {
    {
        "editor.yank_line",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            auto cursor = e.cursor();
            auto line = e.buffer().lineAt(cursor.y());
            scope->env().registers().save(
                standardRegister, std::string{line}, true);
            scope->editor().clearSelection();
        },
    },
    {
        "editor.delete_line",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            auto cursor = e.cursor();
            auto line = e.buffer().lineAt(cursor.y());
            scope->env().registers().save(
                standardRegister, std::string{line}, true);
            e.cursor(deleteLine(e.cursor()));
        },
    },
    {
        "editor.clear_line",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            auto cursor = e.cursor();
            auto line = e.buffer().lineAt(cursor.y());
            scope->env().registers().save(
                standardRegister, std::string{line}, true);
            auto range = CursorRange{home(cursor), end(cursor)};
            erase(range);
        },
    },
    {
        "editor.insert",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            auto event = scope->env().key();
            e.cursor(insert(event.symbol, e.cursor()));
        },
    },
    {
        "editor.split",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            e.cursor(split(e.cursor()));
        },
    },
    {
        "editor.yank", // copy on vim-language
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            auto selection = e.selection();
            if (selection.empty()) {
                scope->env().registers().save(standardRegister,
                                              content(e.cursor()).toString());
            }
            else {
                scope->env().registers().save(standardRegister,
                                              toString(selection));
            }

            e.cursor(selection.begin(), true);
        },
    },
    {
        "editor.erase",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            auto selection = e.selection();
            if (selection.empty()) {
                scope->env().registers().save(standardRegister,
                                              content(e.cursor()).toString());
                e.cursor(erase(e.cursor()));
            }
            else {
                scope->env().registers().save(standardRegister,
                                              toString(selection));
                e.cursor(erase(selection), true);
            }
        },
    },
    {
        // Mostly for ctrl+backspace
        "editor.erase_before_word",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            auto selection = e.selection();
            if (selection.empty()) {
                auto begin = wordBegin(e.cursor());
                e.cursor(erase(CursorRange{begin, e.cursor()}));
            }
            else {
                e.cursor(erase(selection), true);
            }
        },
    },
    {
        "editor.paste_before",
        [](std::shared_ptr<IScope> scope) {
            auto str = scope->env().registers().load(standardRegister);
            auto cursor = scope->editor().cursor();
            if (str.isLine) {
                insert({cursor.buffer(), 0, cursor.y()}, str.value + '\n');
                //                scope->editor().buffer().insert(cursor.y(),
                //                str.value);
            }
            else {
                for (auto c : str.value) {
                    //! Todo: Make efficient if this slows down
                    cursor = insert(c, cursor);
                }
                scope->editor().cursor(left(cursor));
            }
        },
    },
    {
        "editor.paste",
        [](std::shared_ptr<IScope> scope) {
            auto str = scope->env().registers().load(standardRegister);
            auto cursor = scope->editor().cursor();
            if (str.isLine) {
                insert({cursor.buffer(), 0, cursor.y() + 1}, str.value + '\n');
                cursor.y(cursor.y() + 1);
                scope->editor().cursor(cursor);
            }
            else {
                cursor = right(cursor);
                for (auto c : str.value) {
                    //! Todo: Make efficient if this slows down
                    cursor = insert(c, cursor);
                }
                scope->editor().cursor(cursor);
            }
        },
    },
    {
        "editor.join",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            e.cursor(join(e.cursor()));
        },
    },
    {
        "editor.save",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            e.save();
        },
    },
    {
        "editor.copy",
        [](std::shared_ptr<IScope> scope) {
            scope->env().mainWindow().copy(false);
        },
    },
    {
        "editor.cut",
        [](std::shared_ptr<IScope> scope) {
            scope->env().mainWindow().copy(true);
        },
    },
    {
        "editor.copyindentation",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            e.cursor(copyIndentation(e.cursor()));
        },
    },
    {
        "editor.undo",
        [](std::shared_ptr<IScope> scope) { scope->editor().undo(); },
    },
    {
        "editor.undo_major",
        [](std::shared_ptr<IScope> scope) { scope->editor().undoMajor(); },
    },
    {
        "editor.redo",
        [](std::shared_ptr<IScope> scope) { scope->editor().redo(); },
    },
    {
        "editor.build",
        [](std::shared_ptr<IScope> scope) { build(scope); },
    },
    {
        "editor.run",
        [](std::shared_ptr<IScope> scope) { run(scope); },
    },
    {
        "editor.insertmode",
        [](std::shared_ptr<IScope> scope) {
            scope->editor().mode(createInsertMode());
        },
    },
    {
        "editor.normalmode",
        [](std::shared_ptr<IScope> scope) {
            scope->editor().mode(createNormalMode());
        },
    },
    {
        "editor.visualmode",
        [](std::shared_ptr<IScope> scope) {
            scope->editor().mode(createVisualMode(scope->env()));
        },
    },
    {
        "editor.visualblockmode",
        [](std::shared_ptr<IScope> scope) {
            scope->editor().mode(createVisualMode(scope->env(), true));
        },
    },
    {
        "editor.toggle_comment",
        [](std::shared_ptr<IScope> scope) { toggleComments(scope); },
    },
    {"quit", [](auto &&) { quitMedit(); }},
};

CommandList selectionCommands = {
    {
        "editor.select_word",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            auto cursor = e.cursor();
            auto range = CursorRange{cursor, right(wordEnd(cursor))};
            e.selection(range);
        },
    },
    {
        "editor.select_inner_word",
        [](std::shared_ptr<IScope> scope) {
            auto &e = scope->editor();
            auto cursor = e.cursor();
            auto range = CursorRange{wordBegin(cursor), right(wordEnd(cursor))};
            e.selection(range);
        },
    },
};

} // namespace

void addStandardCommands(IScope &scope) {
    auto addCommands = [&scope](auto &list) {
        for (auto &pair : list) {
            scope.addCommand(pair.first, pair.second);
        }
    };

    //    addCommands(navigationCommands);
    addCommands(editorCommands);
    addCommands(selectionCommands);
}

namespace {
StandardCommands create() {
    using Ptr = StandardCommands::EnvPtrT;

#define DEF(name)                                                              \
    commands.namedCommands[#name] = commands.name =                            \
        [](StandardCommands::EnvPtrT env)

    StandardCommands commands{};
    DEF(left) {
        auto &e = env->editor();
        e.cursor(::left(e.cursor()));
    };
    DEF(right) {
        auto &e = env->editor();
        e.cursor(::right(e.cursor()));
    };
    DEF(up) {
        auto &e = env->editor();
        e.cursor(::up(e.cursor()));
    };
    DEF(down) {
        auto &e = env->editor();
        e.cursor(::down(e.cursor()));
    };
    DEF(home) {
        auto &e = env->editor();
        e.cursor(::home(e.cursor()));
    };
    DEF(end) {
        auto &e = env->editor();
        e.cursor(::end(e.cursor()));
    };

    DEF(pageUp) {
        auto &e = env->editor();
        auto c = e.cursor();

        for (size_t i = 0; i < 10; ++i) {
            c = ::up(c);
        }
        e.cursor(c);
    };
    DEF(pageDown) {
        auto &e = env->editor();
        auto c = e.cursor();
        for (size_t i = 0; i < 10; ++i) {
            c = ::down(c);
        }
        e.cursor(c);
    };

    DEF(wordBegin) {
        auto &e = env->editor();
        e.cursor(::wordBegin(e.cursor()));
    };
    DEF(wordEnd) {
        auto &e = env->editor();
        e.cursor(::wordEnd(e.cursor()));
    };

    DEF(switchHeader) {
        auto path = env->project().findSwitchHeader(env->editor().path());
        if (!path.empty()) {
            env->standardCommands().open(env, path, std::nullopt, std::nullopt);
        }
    };

    // -----------------------
    commands.selectInnerWord = [](StandardCommands::EnvPtrT env) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto range =
            CursorRange{::wordBegin(cursor), ::right(::wordEnd(cursor))};
        e.selection(range);
    };
    // ------------------------

    commands.open = [](StandardCommands::EnvPtrT env,
                       std::filesystem::path path,
                       std::optional<int> x,
                       std::optional<int> y) {
        if (path.empty()) {
            return;
        }
        env->mainWindow().open(path, x, y);
    };

    return commands;
}

} // namespace

StandardCommands &StandardCommands::get() {
    static auto sc = create();
    return sc;
}
