
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
#include "script/browsefileinteraction.h"
#include "script/ienvironment.h"
#include "script/indent.h"
#include "script/renamefileinteraction.h"
#include "text/cursorops.h"
#include "text/cursorrange.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "text/utf8charops.h"
#include "togglecomments.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>

int numRepeats(std::shared_ptr<IEnvironment> env) {
    return env->editor().mode().repetitions();
}

namespace {
StandardCommands create() {
    using Ptr = StandardCommands::EnvPtrT;

#define DEF(name)                                                              \
    commands.namedCommands[#name] =                                            \
        commands.name = [=](StandardCommands::EnvPtrT env)

// Repeat the command specified number of times
#define REPEAT                                                                 \
    size_t NUM_REPEATS = numRepeats(env);                                      \
    for (size_t i = 0; i < NUM_REPEATS; ++i)

// Repeat minus one time
#define REPEAT_M1                                                              \
    size_t NUM_REPEATS = numRepeats(env);                                      \
    for (size_t i = 1; i < NUM_REPEATS; ++i)

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

        auto &lines = e.buffer().lines();
        auto repeats = e.mode().repetitions();
        auto str = std::string{};

        for (size_t i = cursor.y();
             i < std::min(cursor.y() + repeats, lines.size());
             ++i) {
            auto line = lines.at(i);
            str += line + "\n";
        }
        str.pop_back();
        env->registers().save(standardRegister, str, true);
        env->editor().clearSelection();
    };

    DEF(delete_line) {
        env->standardCommands().yank_line(env);

        auto &e = env->editor();
        auto cursor = e.cursor();
        auto line = e.buffer().lineAt(cursor.y());
        //        env->registers().save(standardRegister, std::string{line},
        //        true);
        REPEAT {
            deleteLine(e.cursor());
        }
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
            auto str = toString(selection);
            if (str.empty()) {
                return;
            }
            if (str.back() == '\n') {
                str.pop_back();
            }
            env->registers().save(standardRegister, toString(selection), true);
        }

        e.cursor(selection.begin(), true);
    };

    DEF(backspace) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        e.cursor(erase(cursor));
    };

    DEF(erase) {
        auto &e = env->editor();
        auto selection = e.selection();
        if (selection.empty()) {
            auto str = std::string{};

            REPEAT {
                auto cursor = e.cursor();
                if (cursor.x() == 0) {
                    break;
                }
                str = std::string{content(left(cursor)).toString()} + str;
                e.cursor(erase(cursor));
            }
            env->registers().save(standardRegister, str);
        }
        else {
            env->registers().save(standardRegister, toString(selection));
            e.cursor(erase(selection), true);
        }
    };

    DEF(erase_after) {
        auto &e = env->editor();
        auto selection = e.selection();
        if (selection.empty()) {
            auto str = std::string{};

            REPEAT {
                str += content(e.cursor());
                e.cursor(erase(right(e.cursor())));
            }
            env->registers().save(standardRegister, str);
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
            auto str = std::string{};

            REPEAT {
                auto begin = wordBegin(e.cursor());
                auto range = CursorRange{begin, e.cursor()};
                str = toString(selection) + str;
                e.cursor(erase(range));
            };
            env->registers().save(standardRegister, str);
        }
        else {
            env->registers().save(standardRegister, toString(selection));
            e.cursor(erase(selection), true);
        }
    };

    DEF(delete_until_end_of_line) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto end = cursor;

        REPEAT_M1 {
            end.y(end.y() + 1);
        }

        end = ::end(end);
        env->registers().save(standardRegister, toString({cursor, end}));
        erase({cursor, end});
    };

    DEF(change_until_end_of_line) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto end = cursor;

        REPEAT_M1 {
            end.y(end.y() + 1);
        }

        env->standardCommands().insert_mode(env);

        end = ::end(end);
        env->registers().save(standardRegister, toString({cursor, end}));
        erase({cursor, end});
    };

    DEF(delete_word) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto end = cursor;

        if (numRepeats(env) == 1) {

            end = nextWord(cursor, false);
        }

        else {
            REPEAT {
                end = nextWord(end, true);
            }
        }

        env->registers().save(standardRegister, toString({cursor, end}));
        erase({cursor, end});
    };

    DEF(change_word) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        auto end = cursor;

        end = right(wordEnd(cursor), false);
        if (end.y() != cursor.y()) {
            // Specific vim behaviour for the first time
            end = cursor;
        }
        REPEAT_M1 {
            end = right(wordEnd(end));
        }

        env->registers().save(standardRegister, toString({cursor, end}));
        erase({cursor, end});

        env->standardCommands().insert_mode(env);
    };

    DEF(paste_before) {
        auto str = env->registers().load(standardRegister);
        REPEAT {
            auto cursor = env->editor().cursor();
            if (str.isLine) {
                insert({cursor.buffer(), 0, cursor.y()}, str.value + '\n');
            }
            else {
                auto ss = std::istringstream{str.value};
                cursor = insert(cursor, str.value);
                env->editor().cursor(left(cursor));
            }
        }
    };

    DEF(paste) {
        auto str = env->registers().load(standardRegister);
        auto cursor = env->editor().cursor();
        REPEAT {
            if (str.isLine) {
                insert({cursor.buffer(), Cursor::max, cursor.y()},
                       '\n' + str.value);
            }
            else {
                auto ss = std::istringstream{str.value};
                cursor = right(cursor, false);
                cursor = insert(cursor, str.value);
                env->editor().cursor(cursor);
            }
        }

        if (str.isLine) {
            cursor.y(cursor.y() + 1);
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
    DEF(close_brace) {
        auto &e = env->editor();
        auto cursor = e.cursor();
        if (cursor.x() == 0 || cursor.y() == 0) {
            e.cursor(insert('}', cursor));
            return;
        }

        if (!isSpace(content(left(cursor)))) {
            e.cursor(insert('}', cursor));
            return;
        }
        auto left = matchingLeft(cursor, '{', '}', true);
        if (!left) {
            e.cursor(insert('}', cursor));
            return;
        }
        auto indentationLeft = ::indentation(*left);
        auto currentIndentation = ::indentation(cursor);
        auto lineStart = ::home(cursor);
        auto indentationStop = lineStart;
        for (auto c : currentIndentation) {
            indentationStop = right(indentationStop);
        }
        erase(CursorRange{lineStart, indentationStop});
        cursor = insert(lineStart, indentationLeft);
        e.cursor(insert('}', cursor));
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
        env->editor().mode(createVisualMode());
    };
    DEF(visual_block_mode) {
        env->editor().mode(createVisualMode(true));
    };
    DEF(toggle_visual_end) {
        auto &editor = env->editor();
        auto anchor = editor.selectionAnchor();
        if (!anchor) {
            return;
        }
        auto cursor = editor.cursor();
        editor.selectionAnchor(cursor);
        editor.cursor(*anchor);
    };

    DEF(toggle_comment) {
        ::toggleComments(env);
    };
    DEF(quit) {
        quitMedit(env->context());
    };

    DEF(browse_files) {
        beginBrowseFileInteraction(env);
    };

    DEF(close_buffer) {
        auto &buffer = env->editor();
        if (!buffer.closeBuffer()) {
            env->statusMessage(FString{"closing editor..."});
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
        e.mode(createVisualMode(true));
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

    DEF(split_editor) {
        env->mainWindow().splitEditor();
    };

    DEF(close_editor) {
        env->mainWindow().closeEditor();
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
#undef REPEAT
#undef REPEAT_M1

    return commands;
}

} // namespace

StandardCommands &StandardCommands::get() {
    static auto sc = create();
    return sc;
}
