
#include "basichighligting.h"
#include "core/profiler.h"
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

const auto wordListXml = std::vector<std::string_view>{
    "CDATA",
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

const auto wordlistGo = std::vector<std::string_view>{
    "break",    "default",     "func",   "interface", "select",
    "case",     "defer",       "go",     "map",       "struct",
    "chan",     "else",        "goto",   "package",   "switch",
    "const",    "fallthrough", "if",     "range",     "type",
    "continue", "for",         "import", "return",    "var",
};

const auto wordlistPython = std::vector<std::string_view>{
    "False", "class",  "finally", "is",     "return", "None", "continue",
    "for",   "lambda", "try",     "True",   "def",    "from", "nonlocal",
    "while", "and",    "del",     "global", "not",    "with", "as",
    "elif",  "if",     "or",      "yield",  "assert", "else", "import",
    "pass",  "break",  "except",  "in",     "raise",
};

const auto wordlistRust = std::vector<std::string_view>{
    "as",     "break", "const", "else", "enum",   "extern", "false", "fn",
    "for",    "if",    "impl",  "in",   "let",    "loop",   "match", "mod",
    "move",   "mut",   "pub",   "ref",  "return", "self",   "Self",  "static",
    "struct", "super", "trait", "true", "type",   "unsafe", "use",   "where",
    "while",  "async", "await", "dyn",
};

const auto wordlistMake = std::vector<std::string_view>{
    "$@",
    "$<",
    "$^",
    "$+",
    "$$?",
    "$*",
    "all",
    "clean",
    "PHONY",
    "DEFAULT",
    "PRECIOUS",
    "SILENT",
    "EXPORT_ALL_VARIABLES",
    "IGNORE",
    "SECONDARY",
    "MAKE",
    "MAKEFILE_LIST",
    "MAKE_VERSION",
    "CC",
    "CXX",
    "RM",
    "AR",
    "FLINK",
    "LEX",
    "YACC",
    "LD",
    "MAKEINFO",
    "subst",
    "patsubst",
    "strip",
    "findstring",
    "filter",
    "filter-out",
    "sort",
    "word",
    "ifeq",
    "ifneq",
    "ifdef",
    "ifndef",
    "else",
    "endif",
    "INCLUDE_DIRS",
    "INTERMEDIATE",
    "DELETE_ON_ERROR",
};

const auto wordlistCMake = std::vector<std::string_view>{
    "block",
    "break",
    "cmake_host_system_information",
    "cmake_language",
    "cmake_minimum_required",
    "cmake_parse_arguments",
    "cmake_path",
    "cmake_policy",
    "configure_file",
    "continue",
    "else",
    "elseif",
    "endblock",
    "endforeach",
    "endfunction",
    "endif",
    "endmacro",
    "endwhile",
    "execute_process",
    "file",
    "find_file",
    "find_library",
    "find_package",
    "find_path",
    "find_program",
    "foreach",
    "function",
    "get_cmake_property",
    "get_directory_property",
    "get_filename_component",
    "get_property",
    "if",
    "include",
    "include_guard",
    "list",
    "macro",
    "mark_as_advanced",
    "math",
    "message",
    "option",
    "return",
    "separate_arguments",
    "set",
    "set_directory_properties",
    "set_property",
    "site_name",
    "string",
    "unset",
    "variable_watch",
    "while",
    // Project Commands
    "add_compile_definitions",
    "add_compile_options",
    "add_custom_command",
    "add_custom_target",
    "add_definitions",
    "add_dependencies",
    "add_executable",
    "add_library",
    "add_link_options",
    "add_subdirectory",
    "add_test",
    "aux_source_directory",
    "build_command",
    "cmake_file_api",
    "create_test_sourcelist",
    "define_property",
    "enable_language",
    "enable_testing",
    "export",
    "fltk_wrap_ui",
    "get_source_file_property",
    "get_target_property",
    "get_test_property",
    "include_directories",
    "include_external_msproject",
    "include_regular_expression",
    "install",
    "link_directories",
    "link_libraries",
    "load_cache",
    "project",
    "remove_definitions",
    "set_source_files_properties",
    "set_target_properties",
    "set_tests_properties",
    "source_group",
    "target_compile_definitions",
    "target_compile_features",
    "target_compile_options",
    "target_include_directories",
    "target_link_directories",
    "target_link_libraries",
    "target_link_options",
    "target_precompile_headers",
    "target_sources",
    "try_compile",
    "try_run",
    // CTest Commands
    "ctest_build",
    "ctest_configure",
    "ctest_coverage",
    "ctest_empty_binary_directory",
    "ctest_memcheck",
    "ctest_read_custom_files",
    "ctest_run_script",
    "ctest_sleep",
    "ctest_start",
    "ctest_submit",
    "ctest_test",
    "ctest_update",
    "ctest_upload",
    // Deprecated Commands
    "build_name",
    "exec_program",
    "export_library_dependencies",
    "install_files",
    "install_programs",
    "install_targets",
    "load_command",
    "make_directory",
    "output_required_files",
    "qt_wrap_cpp",
    "qt_wrap_ui",
    "remove",
    "subdir_depends",
    "subdirs",
    "use_mangled_mesa",
    "utility_source",
    "variable_requires",
    "write_file",
};

const auto wordlistJson = std::vector<std::string_view>{
    "true",
    "false",
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
    if (isXml(extension)) {
        return &wordListXml;
    }
    if (isJs(extension)) {
        return &wordlistJavascript;
    }
    if (isPython(extension)) {
        return &wordlistPython;
    }
    if (isGo(extension)) {
        return &wordlistGo;
    }
    if (isRust(extension)) {
        return &wordlistRust;
    }
    if (isMake(extension)) {
        return &wordlistMake;
    }
    if (isCmakeLists(extension)) {
        return &wordlistCMake;
    }
    if (isJson(extension)) {
        return &wordlistJson;
    }
    return nullptr;
}

} // namespace

BasicHighlighting::BasicHighlighting() = default;

BasicHighlighting::~BasicHighlighting() = default;

bool BasicHighlighting::shouldEnable(std::filesystem::path) {
    return true;
}

bool BasicHighlighting::highlight(Buffer &buffer) {
    return highlightStatic(buffer);
}

bool BasicHighlighting::highlightStatic(Buffer &buffer) {
    auto duration = ProfileDuration{};
    if (!shouldEnable(buffer.path())) {
        return false;
    }
    if (buffer.empty()) {
        return true;
    }

    decltype(wordListCpp) *wordList = [&buffer] {
        if (auto f = buffer.file()) {
            return getWordList(f->path());
        }
        return (decltype(wordListCpp) *){};
    }();

    if (!wordList) {
        return true;
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

    buffer.emitChangeSignal();
    return true;
}
