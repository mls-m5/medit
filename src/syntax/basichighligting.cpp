
#include "basichighligting.h"
#include "files/extensions.h"
#include "syntax/palette.h"
#include "text/cursorrangeops.h"
#include "text/fchar.h"
#include "text/utf8char.h"
#include "text/words.h"
#include <array>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <locale>
#include <string>
#include <string_view>
#include <vector>

namespace {
//! Todo: Extract this so it can be customized
const auto wordListCpp = std::vector<std::string_view>{
    "int",      "double",   "char",   "signed",    "long",
    "unsigned", "void",     "short",  "bool",      "auto",
    "include",  "struct",   "class",  "if",        "else",
    "switch",   "case",     "public", "private",   "namespace",
    "typename", "template", "enum",   "default",   "thread_local",
    "true",     "false",    "this",   "constexpr",
};

const auto wordListHtml = std::vector<std::string_view>{
    "a",        "abbr",     "acronym",  "address",    "applet",     "area",
    "article",  "aside",    "audio",    "b",          "base",       "basefont",
    "bdi",      "bdo",      "big",      "blockquote", "body",       "br",
    "button",   "canvas",   "caption",  "center",     "cite",       "code",
    "col",      "colgroup", "data",     "datalist",   "dd",         "del",
    "details",  "dfn",      "dialog",   "dir",        "div",        "dl",
    "dt",       "em",       "embed",    "fieldset",   "figcaption", "figure",
    "font",     "footer",   "form",     "frame",      "frameset",   "h1",
    "h2",       "h3",       "h4",       "h5",         "h6",         "head",
    "header",   "hr",       "html",     "i",          "iframe",     "img",
    "input",    "ins",      "kbd",      "label",      "legend",     "li",
    "link",     "main",     "map",      "mark",       "meta",       "meter",
    "nav",      "noframes", "noscript", "object",     "ol",         "optgroup",
    "option",   "output",   "p",        "param",      "picture",    "pre",
    "progress", "q",        "rp",       "rt",         "ruby",       "s",
    "samp",     "script",   "section",  "select",     "small",      "source",
    "span",     "strike",   "strong",   "style",      "sub",        "summary",
    "sup",      "svg",      "table",    "tbody",      "td",         "template",
    "textarea", "tfoot",    "th",       "thead",      "time",       "title",
    "tr",       "track",    "tt",       "u",          "ul",         "var",
    "video",    "wbr",
};

const auto wordlistJavascript = std::vector<std::string_view>{
    "abstract",     "arguments", "await",    "boolean",    "break",
    "byte",         "case",      "catch",    "char",       "class",
    "const",        "continue",  "debugger", "default",    "delete",
    "do",           "double",    "else",     "enum",       "eval",
    "export",       "extends",   "false",    "final",      "finally",
    "float",        "for",       "function", "goto",       "if",
    "implements",   "import",    "in",       "instanceof", "int",
    "interface",    "let",       "long",     "native",     "new",
    "null",         "package",   "private",  "protected",  "public",
    "return",       "short",     "static",   "super",      "switch",
    "synchronized", "this",      "throw",    "throws",     "transient",
    "true",         "try",       "typeof",   "var",        "void",
    "volatile",     "while",     "with",     "yield",
};

template <typename ListT>
void highlightWord(CursorRange word, const ListT &list) {
    for (auto &w : list) {
        if (word == w) {
            ::format(word, Palette::statement);
            break;
        }
    }
}

const std::vector<std::string_view> *getWordList(
    const std::filesystem::path &extension) {
    if (isCpp(extension)) {
        return &wordListCpp;
    }
    if (isHtml(extension)) {
        return &wordListHtml;
    }
    if (isJs(extension)) {
        return &wordlistJavascript;
    }
    if (isPython(extension)) {
        return &wordlistJavascript;
    }
    return nullptr;
}

} // namespace

BasicHighlighting::BasicHighlighting() = default;

BasicHighlighting::~BasicHighlighting() = default;

bool BasicHighlighting::shouldEnable(filesystem::path) {
    return true;
}

void BasicHighlighting::highlight(Buffer &buffer) {
    highlightStatic(buffer);
}

void BasicHighlighting::highlightStatic(Buffer &buffer) {
    if (buffer.empty()) {
        return;
    }

    decltype(wordListCpp) *wordList = [&buffer] {
        if (auto f = buffer.file()) {
            return getWordList(f->path());
        }
        return (decltype(wordListCpp) *){};
    }();

    if (!wordList) {
        return;
    }

    format(all(buffer), Palette::standard);

    {
        // Find function names
        auto prev = CursorRange{buffer.begin(), buffer.begin()};

        for (auto word : Words(buffer)) {
            auto first =
                content(word.begin()).at(0); // Only match the first part
            if (first == '(' or first == '.') {
                if (prev.empty()) {
                    continue;
                }

                if (std::isalpha(content(prev.begin()).at(0), std::locale())) {
                    format(prev, Palette::identifier);
                }
            }
            prev = word;
        }
    }

    for (auto word : Words(buffer)) {
        highlightWord(word, *wordList);
    }

    //     Identify comment
    for (size_t y = 0; y < buffer.lines().size(); ++y) {
        auto &line = buffer.lineAt(y);
        for (size_t x = 1; x < line.size(); ++x) {

            if (line.at(x - 1).c == '/' && line.at(x).c == '/') {
                --x;

                auto range = CursorRange{buffer, {x, y}, {10000, y}};
                format(range, Palette::comment);
                break;
            }
        }

        if (!line.empty()) {
            if (line.front().c == '#') {
                auto range = CursorRange{buffer, {0, y}, {10000, y}};
                format(range, Palette::comment);
            }
        }
    }

    // Identify strings
    for (size_t y = 0; y < buffer.lines().size(); ++y) {
        FChar startChar = '"';
        auto &line = buffer.lineAt(y);
        for (size_t x = 0; x < line.size(); ++x) {
            auto c = line.at(x).c;
            if (c == '"' || c == '\'') {
                size_t begin = x;
                startChar = line.at(x);
                ++x;
                for (; x < line.size(); ++x) {
                    if (line.at(x).c == startChar) {
                        ++x;
                        break;
                    }
                }
                size_t end = x;

                auto range = CursorRange{buffer, {begin, y}, {end, y}};
                //                std::cout << "string: " << range << std::endl;
                format(range, Palette::string);
            }
        }
    }

    buffer.isColorsOld(false);
}
