
#include "basichighligting.h"
#include "script/ienvironment.h"
#include "span.h"
#include "syntax/palette.h"
#include "text/cursorops.h"
#include "text/cursorrangeops.h"
#include "text/words.h"
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

void BasicHighlighting::highlight(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();
    auto &buffer = editor.buffer();

    if (buffer.empty()) {
        return;
    }

    for (auto c : buffer) {
        if (c) {
            c->f = IPalette::standard;
        }
    }

    for (auto word : Words(buffer)) {
        highlightWord(word);
    }

    // Identify comment
    for (size_t i = 0; i < buffer.lines().size(); ++i) {
        auto &line = buffer.lineAt(i);
        for (size_t i = 1; i < line.size(); ++i) {

            if (line.at(i - 1).c == '/' && line.at(i).c == '/') {
                --i;
                for (; i < line.size(); ++i) {
                    auto &c = line.at(i);
                    c.f = IPalette::comment;
                }
                break;
            }
        }

        if (!line.empty()) {
            if (line.front().c == '#') {
                for (auto &c : line) {
                    c.f = IPalette::comment;
                }
            }
        }
    }
}

void BasicHighlighting::update(const IPalette &palette) {}
