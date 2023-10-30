
#include "standardcommands.h"
#include "core/coreenvironment.h"
#include "core/registers.h"
#include "files/project.h"
#include "main.h"
#include "modes/insertmode.h"
#include "modes/normalmode.h"
#include "modes/visualmode.h"
#include "plugin/build.h"
#include "plugin/idebugger.h"
#include "plugin/run.h"
#include "plugin/rundebug.h"
#include "renameinteraction.h"
#include "saveinteraction.h"
#include "script/ienvironment.h"
#include "script/indent.h"
#include "script/renamefileinteraction.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "togglecomments.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <functional>
#include <map>
#include <sstream>
#include <string>

namespace {
StandardCommands create() {
    using Ptr = StandardCommands::EnvPtrT;

#define DEF(name)                                                              \
    commands.namedCommands[#name] =                                            \
        commands.name = [=](StandardCommands::EnvPtrT env)

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

    DEF(page_up) {
        auto &e = env->editor();
        auto c = e.cursor();

        for (size_t i = 0; i < 10; ++i) {
            c = ::up(c);
        }
        e.cursor(c);
    };
    DEF(page_down) {
        auto &e = env->editor();
        auto c = e.cursor();
        for (size_t i = 0; i < 10; ++i) {
            c = ::down(c);
        }
        e.cursor(c);
    };

    DEF(word_begin) {
        auto &e = env->editor();
        e.cursor(::wordBegin(e.cursor()));
    };
    DEF(word_end) {
        auto &e = env->editor();
        e.cursor(::wordEnd(e.cursor()));
    };

    DEF(switch_header) {
        auto path = env->project().findSwitchHeader(env->editor().path());
        if (!path.empty()) {
            env->standardCommands().open(env, path, std::nullopt, std::nullopt);
        }
    };

    // ------- Edit ----------

    DEF(yank_line) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto line = e.buffer().lineAt(cursor.y());
        env->registers().save(standardRegister, std::string{line}, true);
        env->editor().clearSelection();
    };

    DEF(delete_line) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto line = e.buffer().lineAt(cursor.y());
        env->registers().save(standardRegister, std::string{line}, true);
        deleteLine(e.cursor());
        e.cursor({e.buffer(), cursor.x(), cursor.y()});
    };

    DEF(clear_line) {
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

    DEF(yank_block) {
        // Used in visual block
        // Maybe find some way to merge this with "yank"
        auto &e = env->editor();
        auto selection = e.selection();
        if (selection.empty()) {
            env->registers().save(
                standardRegister, content(e.cursor()).toString(), true);
        }
        else {
            env->registers().save(standardRegister, toString(selection), true);
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

    DEF(erase_before_word) {
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

    DEF(paste_before) {
        auto str = env->registers().load(standardRegister);
        auto cursor = env->editor().cursor();
        auto oldCursor = cursor;
        if (str.isLine) {
            insert({cursor.buffer(), 0, cursor.y()}, str.value + '\n');
        }
        else {
            auto ss = std::istringstream{str.value};
            cursor = insert(cursor, str.value);
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
            auto ss = std::istringstream{str.value};
            cursor = right(cursor, false);
            cursor = insert(cursor, str.value);
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
        saveInteraction(env);
    };
    DEF(copy) {
        env->mainWindow().copy(false);
    };
    DEF(cut) {
        env->mainWindow().copy(true);
    };
    DEF(copy_indentation) {
        auto &e = env->editor();
        e.cursor(copyIndentation(e.cursor()));
    };
    DEF(undo) {
        env->editor().undo();
    };
    DEF(undo_major) {
        env->editor().undoMajor();
    };
    DEF(redo) {
        env->editor().redo();
    };
    DEF(build) {
        ::build(env);
    };
    DEF(run) {
        ::build(env);
        ::run(env);
    };
    DEF(insert_mode) {
        env->editor().mode(createInsertMode());
    };
    DEF(normal_mode) {
        env->editor().mode(createNormalMode());
    };
    DEF(visual_mode) {
        env->editor().mode(createVisualMode(*env));
    };
    DEF(visual_block_mode) {
        env->editor().mode(createVisualMode(*env, true));
    };
    DEF(toggle_comment) {
        ::toggleComments(env);
    };
    DEF(quit) {
        quitMedit(env->context());
    };

    DEF(close_buffer) {
        auto &buffer = env->editor();
        if (!buffer.closeBuffer()) {
            quitMedit(env->context());
            return;
        }
        env->mainWindow().updateTitle();
    };

    DEF(indent) {
        indent(env);
    };
    DEF(deindent) {
        deindent(env);
    };

    // -----------------------
    DEF(select_word) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto range = CursorRange{cursor, right(wordEnd(cursor))};
        e.selection(range);
    };

    DEF(select_all) {
        auto &e = env->editor();
        e.mode(createVisualMode(*env, true));
        e.selection(all(e.buffer()));
    };

    DEF(rename_symbol) {
        beginRenameInteraction(env);
    };
    DEF(new_file) {
        env->editor().buffer(env->core().files().create());
    };

    DEF(rename_file) {
        beginRenameFileInteraction(env);
    };

    DEF(select_inner_word) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto range =
            CursorRange{::wordBegin(cursor), ::right(::wordEnd(cursor))};
        e.selection(range);
    };

    DEF(select_inner_paren) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto range = inner(cursor, "(", ")");
        range.endPosition(left(range.end()));
        e.selection(range);
    };

    DEF(select_around_paren) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto range = inner(cursor, "(", ")");
        range.beginPosition(left(range.begin()));
        e.selection(range);
    };

    DEF(command_palette) {
        env->mainWindow().showCommandPalette();
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

    // --- debugger ----------------------

#define MAP_DEBUG(x, y, ...)                                                   \
    DEF(x) {                                                                   \
        if (env->core().debugger()) {                                          \
            env->core().debugger()->y(__VA_ARGS__);                            \
        }                                                                      \
    };

    DEF(debug_run) {
        debug(env);
    };
    //    MAP_DEBUG(debug_run, run);
    MAP_DEBUG(debug_pause, pause);
    MAP_DEBUG(debug_stop, stop);

    MAP_DEBUG(debug_continue, cont);
    MAP_DEBUG(debug_step_into, stepInto);
    MAP_DEBUG(debug_step_over, stepOver);
    MAP_DEBUG(debug_step_out, stepOut);

    MAP_DEBUG(debug_step_out, stepOut);

    auto currentFileLocation = [](IEnvironment &env) {
        auto source = SourceLocation{};
        source.path = env.editor().path();
        source.position = env.editor().cursor();
        return source;
    };

    MAP_DEBUG(
        debug_toggle_breakpoint, toggleBreakpoint, currentFileLocation(*env));

#undef MAP_DEBUG
#undef DEF

    return commands;
}

} // namespace

StandardCommands &StandardCommands::get() {
    static auto sc = create();
    return sc;
}
