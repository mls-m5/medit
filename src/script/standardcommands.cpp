
#include "standardcommands.h"
#include "core/registers.h"
#include "files/project.h"
#include "main.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "modes/visualmode.h"
#include "plugin/build.h"
#include "plugin/run.h"
#include "renameinteraction.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "togglecomments.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <functional>
#include <map>
#include <string>

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

    // ------- Edit ----------

    DEF(yankLine) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto line = e.buffer().lineAt(cursor.y());
        env->registers().save(standardRegister, std::string{line}, true);
        env->editor().clearSelection();
    };

    DEF(deleteLine) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto line = e.buffer().lineAt(cursor.y());
        env->registers().save(standardRegister, std::string{line}, true);
        e.cursor(deleteLine(e.cursor()));
    };

    DEF(clearLine) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto line = e.buffer().lineAt(cursor.y());
        env->registers().save(standardRegister, std::string{line}, true);
        auto range = CursorRange{home(cursor), end(cursor)};
        erase(range);
    };

    DEF(insert) {
        auto &e = env->editor();
        auto event = env->key();
        e.cursor(insert(event.symbol, e.cursor()));
    };
    DEF(insert) {
        auto &e = env->editor();
        auto event = env->key();
        e.cursor(insert(event.symbol, e.cursor()));
    };
    DEF(split) {
        auto &e = env->editor();
        e.cursor(split(e.cursor()));
    };

    DEF(yank) {
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
    };

    DEF(erase) {
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
    };

    DEF(eraseBeforeWord) {
        // Mostly for ctrl+backspace
        auto &e = env->editor();
        auto selection = e.selection();
        if (selection.empty()) {
            auto begin = wordBegin(e.cursor());
            e.cursor(erase(CursorRange{begin, e.cursor()}));
        }
        else {
            e.cursor(erase(selection), true);
        }
    };

    DEF(pasteBefore) {
        auto str = env->registers().load(standardRegister);
        auto cursor = env->editor().cursor();
        if (str.isLine) {
            insert({cursor.buffer(), 0, cursor.y()}, str.value + '\n');
        }
        else {
            for (auto c : str.value) {
                //! Todo: Make efficient if this slows down
                cursor = insert(c, cursor);
            }
            env->editor().cursor(left(cursor));
        }
    };

    DEF(paste) {
        auto str = env->registers().load(standardRegister);
        auto cursor = env->editor().cursor();
        if (str.isLine) {
            insert({cursor.buffer(), 0, cursor.y() + 1}, str.value + '\n');
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
    };
    DEF(join) {
        auto &e = env->editor();
        e.cursor(join(e.cursor()));
    };

    DEF(format) {
        env->mainWindow().format();
        auto &e = env->editor();
        e.save();
    };
    DEF(save) {
        env->editor().mode(createNormalMode());
        auto &e = env->editor();
        e.save();
    };
    DEF(copy) {
        env->mainWindow().copy(false);
    };
    DEF(cut) {
        env->mainWindow().copy(true);
    };
    DEF(copyIndentation) {
        auto &e = env->editor();
        e.cursor(copyIndentation(e.cursor()));
    };
    DEF(undo) {
        env->editor().undo();
    };
    DEF(undoMajor) {
        env->editor().undoMajor();
    };
    DEF(redo) {
        env->editor().redo();
    };
    DEF(build) {
        ::build(env);
    };
    DEF(run) {
        ::run(env);
    };
    DEF(insertMode) {
        env->editor().mode(createInsertMode());
    };
    DEF(normalMode) {
        env->editor().mode(createNormalMode());
    };
    DEF(visualMode) {
        env->editor().mode(createVisualMode(*env));
    };
    DEF(visualBlockMode) {
        env->editor().mode(createVisualMode(*env, true));
    };
    DEF(toggleComment) {
        ::toggleComments(env);
    };
    DEF(quit) {
        quitMedit();
    };

    // -----------------------
    DEF(selectWord) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto range = CursorRange{cursor, right(wordEnd(cursor))};
        e.selection(range);
    };

    DEF(selectAll) {
        auto &e = env->editor();
        e.mode(createVisualMode(*env, true));
        auto begin = e.buffer().begin();
        auto end = e.buffer().end();
        e.selection({begin, end});
    };

    DEF(renameSymbol) {
        beginRenameInteraction(env);
    };

    DEF(selectInnerWord) {
        //    commands.selectInnerWord = [](StandardCommands::EnvPtrT env) {
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
