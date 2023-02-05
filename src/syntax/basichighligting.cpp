
#include "basichighligting.h"
#include "core/plugins.h"
#include "script/iscope.h"
#include "span.h"
#include "syntax/palette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "text/words.h"
#include "views/editor.h"
#include <array>
#include <string>

namespace {
//! Todo: Extract this so it can be customized
const auto wordList = std::array<std::string, 28>{
    "int",          "double",    "char",     "signed",   "long",    "unsigned",
    "void",         "short",     "bool",     "auto",     "include", "struct",
    "class",        "if",        "else",     "switch",   "case",    "public",
    "private",      "namespace", "typename", "template", "enum",    "default",
    "thread_local", "true",      "false",    "this",
};

void highlightWord(CursorRange word) {
    for (auto &w : wordList) {
        if (word == w) {
            ::format(word, IPalette::statement);
            break;
        }
    }
}

} // namespace

BasicHighlighting::BasicHighlighting() = default;

BasicHighlighting::~BasicHighlighting() = default;

bool BasicHighlighting::shouldEnable(filesystem::path) {
    return true;
}

void BasicHighlighting::highlight(std::shared_ptr<IScope> scope) {
    auto &editor = scope->editor();
    auto &buffer = editor.buffer();
    highlightStatic(buffer);
}

void BasicHighlighting::highlightStatic(Buffer &buffer) {
    if (buffer.empty()) {
        return;
    }

    format(all(buffer), IPalette::standard);

    for (auto word : Words(buffer)) {
        highlightWord(word);
    }

    //     Identify comment
    for (size_t y = 0; y < buffer.lines().size(); ++y) {
        auto &line = buffer.lineAt(y);
        for (size_t x = 1; x < line.size(); ++x) {

            if (line.at(x - 1).c == '/' && line.at(x).c == '/') {
                --x;

                auto range = CursorRange{buffer, {x, y}, {10000, y}};
                format(range, IPalette::comment);
                break;
            }
        }

        if (!line.empty()) {
            if (line.front().c == '#') {
                auto range = CursorRange{buffer, {0, y}, {10000, y}};
                format(range, IPalette::comment);
            }
        }
    }

    buffer.isColorsOld(false);
}

// void BasicHighlighting::update(const IPalette &palette) {}

void BasicHighlighting::registerPlugin() {
    registerHighlighting<BasicHighlighting>();
}
