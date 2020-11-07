
#include "colorize.h"
#include "span.h"
#include <string>
#include <vector>

namespace {
//! Todo: Extract this so it can be customized
const auto wordList = std::vector<std::string>{
    "int",   "double", "char", "signed",  "long",    "unsigned",  "void",
    "short", "bool",   "auto", "include", "struct",  "class",     "if",
    "else",  "switch", "case", "public",  "private", "namespace",
};

void colorizeWord(span<FChar> line) {
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
                c.f = 1;
            }
            return;
        }
    }
    for (auto &c : line) {
        c.f = 0;
    }
}

int charType(const Utf8Char &c) {
    return isalnum(c.front());
}

} // namespace

void colorize(FString &line) {
    if (line.empty()) {
        return;
    }
    size_t lastWord = 0;
    int lastType = charType(line.at(0).c);
    for (size_t i = 1; i < line.size(); ++i) {
        int type = charType(line.at(i).c);

        if (type != lastType) {
            colorizeWord(span<FChar>{&line.at(lastWord), &line.at(i)});
            lastWord = i;
            lastType = type;
        }
    }
    //    for (auto &c : line) {
    //        if (c.c == 'a') {
    //            c.f = 1;
    //        }
    //        else {
    //            c.f = 0;
    //        }
    //    }
}
