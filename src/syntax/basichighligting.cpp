
#include "basichighligting.h"
#include "span.h"
#include "syntax/palette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <string>
#include <vector>

struct BasicHighlighting::Format {
    FormatType textFormat = 0;
    FormatType statement = 0;
};

namespace {
//! Todo: Extract this so it can be customized
const auto wordList = std::vector<std::string>{
    "int",   "double", "char", "signed",  "long",    "unsigned",  "void",
    "short", "bool",   "auto", "include", "struct",  "class",     "if",
    "else",  "switch", "case", "public",  "private", "namespace",
};

void highlightWord(CursorRange word) {
    for (auto &w : wordList) {
        if (word == w) {
            break;
        }
    }
}

} // namespace

void BasicHighlighting::colorize(Buffer &buffer) {
    for (auto c : buffer) {
        if (c) {
            c->f = _format->textFormat;
        }
    }

    for (auto c = wordEnd(buffer.begin()); c < buffer.end();
         c = wordEnd(right(c))) {
        auto b = wordBegin(c);

        auto range = CursorRange{b, right(c)};

        highlightWord(range);
    }
}

BasicHighlighting::BasicHighlighting() : _format(std::make_unique<Format>()) {}

BasicHighlighting::~BasicHighlighting() = default;

bool BasicHighlighting::shouldEnable(filesystem::path) {
    return true;
}

void BasicHighlighting::highlight(Editor &editor) {
    colorize(editor.buffer());
}

void BasicHighlighting::update(const IPalette &palette) {
    _format->textFormat = palette.getFormat("text");
    _format->statement = palette.getFormat("def:statement");
}
