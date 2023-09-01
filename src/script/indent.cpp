#include "indent.h"
#include "files/project.h"
#include "script/ienvironment.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"

void indent(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();

    const auto selection = editor.selection();
    const auto selectionStart = selection.begin().y();
    const auto selectionEnd = selection.end().y();

    const auto indent = FString{env->project().indentWidth(), ' '};

    for (size_t i = selectionStart; i <= selectionEnd; ++i) {
        insert(Cursor{editor.buffer(), 0, i}, indent);
    }

    editor.buffer().history().markMajor();
}

void deindent(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();

    const auto selection = editor.selection();
    const auto selectionStart = selection.begin().y();
    const auto selectionEnd = selection.end().y();

    auto &buffer = editor.buffer();

    const auto indentWidth = env->project().indentWidth();

    const auto space = Utf8Char{' '};

    /// Only remove characters if they are spaces
    for (size_t i = selectionStart; i <= selectionEnd; ++i) {
        auto iw = indentWidth;
        auto line = buffer.lineAt(i);
        for (int j = 0; j < iw; ++j) {
            if (line.at(j) != space) {
                iw = j;
                break;
            }
        }
        erase(CursorRange{buffer, {0, i}, {iw, i}});
    }

    editor.buffer().history().markMajor();
}
