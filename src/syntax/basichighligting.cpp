
#include "basichighligting.h"
#include "span.h"
#include "syntax/palette.h"
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

void colorizeWord(span<FChar> line, const BasicHighlighting::Format &f) {
    for (auto &word : wordList) {
        if (word.size() != line.size()) {
            continue;
        }

        size_t i = 0;

        for (; i < line.size() && i < word.size(); ++i) {
            if (!((line.begin() + i)->c == word.at(i))) {
                break;
            }
        }

        if (i == word.size()) {
            for (auto &c : line) {
                c.f = f.statement;
            }
            return;
        }
    }
    for (auto &c : line) {
        c.f = f.textFormat;
    }
}

int charType(const Utf8Char &c) {
    return isalnum(c.front());
}

} // namespace
void BasicHighlighting::colorize(FString &line) {
    if (line.empty()) {
        return;
    }
    size_t lastWord = 0;
    int lastType = charType(line.at(0).c);
    for (size_t i = 1; i < line.size(); ++i) {
        int type = charType(line.at(i).c);

        if (type != lastType) {
            colorizeWord(span<FChar>{&line.at(lastWord), &line.at(i)},
                         *_format);
            lastWord = i;
            lastType = type;
        }
    }
}

void BasicHighlighting::colorize(Buffer &buffer) {
    for (size_t i = 0; i < buffer.lines().size(); ++i) {
        colorize(buffer.lineAt(i));
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

void BasicHighlighting::update(const Palette &palette) {
    _format->textFormat = palette.getFormat("text");
    _format->statement = palette.getFormat("def:statement");
}
