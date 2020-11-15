
#include "basichighligting.h"
#include "span.h"
#include "syntax/palette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "views/editor.h"
#include <string>
#include <vector>

namespace {
//! Todo: Extract this so it can be customized
const auto wordList = std::vector<std::string>{
    "int",   "double", "char", "signed",  "long",    "unsigned",  "void",
    "short", "bool",   "auto", "include", "struct",  "class",     "if",
    "else",  "switch", "case", "public",  "private", "namespace",
};

void highlightWord(CursorRange word, const IPalette::BasicPalette &palette) {
    for (auto &w : wordList) {
        if (word == w) {
            ::format(word, palette.statement);
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

void BasicHighlighting::highlight(Editor &editor) {
    auto &buffer = editor.buffer();

    for (auto c : buffer) {
        if (c) {
            c->f = _palette.standard;
        }
    }

    for (auto c = wordEnd(buffer.begin()); c < buffer.end();
         c = wordEnd(right(c))) {
        auto b = wordBegin(c);

        auto range = CursorRange{b, right(c)};

        highlightWord(range, _palette);
    }

    for (size_t i = 0; i < buffer.lines().size(); ++i) {
        auto &line = buffer.lineAt(i);
        for (size_t i = 1; i < line.size(); ++i) {

            if (line.at(i - 1).c == '/' && line.at(i).c == '/') {
                --i;
                for (; i < line.size(); ++i) {
                    auto &c = line.at(i);
                    c.f = _palette.comment;
                }
                break;
            }
        }
        //        if (line.size() >= 2) {

        //            if (line.at(0).c == '/' && line.at(1).c == '/') {
        //                for (auto &c : line) {
        //                    c.f = _palette.comment;
        //                }
        //            }
        //        }
    }
}

void BasicHighlighting::update(const IPalette &palette) {
    _palette = palette.palette();
}
