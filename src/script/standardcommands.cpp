
#include "standardcommands.h"
#include "core/registers.h"
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
#include "togglecomments.h"
#include "views/editor.h"
#include "clang/clangnavigation.h" // extract to plugin later
#include <functional>
#include <map>

namespace {
using CommandList =
    std::map<std::string, std::function<void(std::shared_ptr<IEnvironment>)>>;

CommandList navigationCommands = {

    {
        "editor.left",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.cursor(left(e.cursor()));
        },
    },
    {
        "editor.right",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.cursor(right(e.cursor()));
        },
    },
    {
        "editor.up",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
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
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            auto cursor = e.cursor();
            cursor.y(e.cursor().y() + 1);
            e.cursor(cursor);
        },
    },
    {
        "editor.word_begin",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.cursor(wordBegin(e.cursor()));
        },
    },
    {
        "editor.word_end",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.cursor(wordEnd(e.cursor()));
        },
    },
    {
        "editor.switch_header",
        [](std::shared_ptr<IEnvironment> env) {
            auto path = env->project().findSwitchHeader(env->editor().path());
            if (!path.empty()) {
                env->set("path", path.string());
                env->run(parse("editor.open"));
            }
        },
    },
    {
        "editor.home",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.cursor(home(e.cursor()));
        },
    },
    {
        "editor.end",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.cursor(end(e.cursor()));
        },
    },
};

CommandList editorCommands = {
    {
        "editor.yank_line",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            auto cursor = e.cursor();
            auto line = e.buffer().lineAt(cursor.y());
            env->registers().save(standardRegister, std::string{line}, true);
            env->editor().clearSelection();
        },
    },
    {
        "editor.delete_line",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            auto cursor = e.cursor();
            auto line = e.buffer().lineAt(cursor.y());
            env->registers().save(standardRegister, std::string{line}, true);
            e.cursor(deleteLine(e.cursor()));
        },
    },
    {
        "editor.clear_line",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            auto cursor = e.cursor();
            auto line = e.buffer().lineAt(cursor.y());
            env->registers().save(standardRegister, std::string{line}, true);
            auto range = CursorRange{home(cursor), end(cursor)};
            erase(range);
        },
    },
    {
        "editor.insert",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            auto event = env->key();
            e.cursor(insert(event.symbol, e.cursor()));
        },
    },
    {
        "editor.split",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.cursor(split(e.cursor()));
        },
    },
    {
        "editor.yank", // copy on vim-language
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            auto selection = e.selection();
            if (selection.empty()) {
                env->registers().save(standardRegister,
                                      content(e.cursor()).toString());
            }
            else {
                env->registers().save(standardRegister, toString(selection));
            }

            e.cursor(selection.begin(), true);
        },
    },
    {
        "editor.erase",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            auto selection = e.selection();
            if (selection.empty()) {
                env->registers().save(standardRegister,
                                      content(e.cursor()).toString());
                e.cursor(erase(e.cursor()));
            }
            else {
                env->registers().save(standardRegister, toString(selection));
                e.cursor(erase(selection), true);
            }
        },
    },
    {
        "editor.paste_before",
        [](std::shared_ptr<IEnvironment> env) {
            auto str = env->registers().load(standardRegister);
            auto cursor = env->editor().cursor();
            if (str.isLine) {
                env->editor().buffer().insert(cursor.y(), str.value);
            }
            else {
                for (auto c : str.value) {
                    //! Todo: Make efficient if this slows down
                    cursor = insert(c, cursor);
                }
                env->editor().cursor(left(cursor));
            }
        },
    },
    {
        "editor.paste",
        [](std::shared_ptr<IEnvironment> env) {
            auto str = env->registers().load(standardRegister);
            auto cursor = env->editor().cursor();
            if (str.isLine) {
                env->editor().buffer().insert(cursor.y() + 1, str.value);
                cursor.y(cursor.y() + 1);
                env->editor().cursor(cursor);
            }
            else {
                cursor = right(cursor);
                for (auto c : str.value) {
                    //! Todo: Make efficient if this slows down
                    cursor = insert(c, cursor);
                }
                env->editor().cursor(cursor);
            }
        },
    },
    {
        "editor.join",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.cursor(join(e.cursor()));
        },
    },
    {
        "editor.save",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.save();
        },
    },
    {
        "editor.copyindentation",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            e.cursor(copyIndentation(e.cursor()));
        },
    },
    {
        "editor.undo",
        [](std::shared_ptr<IEnvironment> env) { env->editor().undo(); },
    },
    {
        "editor.redo",
        [](std::shared_ptr<IEnvironment> env) { env->editor().redo(); },
    },
    {
        "editor.build",
        [](std::shared_ptr<IEnvironment> env) { build(env); },
    },
    {
        "editor.insertmode",
        [](std::shared_ptr<IEnvironment> env) {
            env->editor().mode(createInsertMode());
        },
    },
    {
        "editor.normalmode",
        [](std::shared_ptr<IEnvironment> env) {
            env->editor().mode(createNormalMode());
        },
    },
    {
        "editor.visualmode",
        [](std::shared_ptr<IEnvironment> env) {
            env->editor().mode(createVisualMode());
        },
    },
    {
        "editor.toggle_comment",
        [](std::shared_ptr<IEnvironment> env) { toggleComments(env); },
    },
    {"quit", [](auto &&) { quitMedit(); }},
};

CommandList selectionCommands = {
    {
        "editor.select_word",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            auto cursor = e.cursor();
            auto range = CursorRange{cursor, right(wordEnd(cursor))};
            e.selection(range);
        },
    },
    {
        "editor.select_inner_word",
        [](std::shared_ptr<IEnvironment> env) {
            auto &e = env->editor();
            auto cursor = e.cursor();
            auto range = CursorRange{wordBegin(cursor), right(wordEnd(cursor))};
            e.selection(range);
        },
    },
};
} // namespace

void addStandardCommands(IEnvironment &env) {
    auto addCommands = [&env](auto &list) {
        for (auto &pair : list) {
            env.addCommand(pair.first, pair.second);
        }
    };

    addCommands(navigationCommands);
    addCommands(editorCommands);
    addCommands(selectionCommands);
}
