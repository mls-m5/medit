#include "plugin/genericformat.h"
#include "core/os.h"
#include "files/config.h"
#include "files/extensions.h"
#include "views/editor.h"
#include <array>
#include <filesystem>
#include <string>

namespace {

std::string findLatestClangFormat() {
    for (int i = 30; i > 5; --i) {
        auto command = "clang-format-" + std::to_string(i);
        if (hasCommand(command)) {
            return command;
        }
    }

    if (hasCommand("clang-format")) {
        return "clang-format";
    }

    return {};
}

bool formatClang(std::filesystem::path path, Editor &editor) {
    if (!isCpp(path) && !isC(path) && !isJs(path) && !isJava(path) &&
        !isCSharp(path)) {
        return false;
    }

    auto clang = findLatestClangFormat();
    if (clang.empty()) {
        // TODO: Print error message
        return false;
    }
    editor.save();
    runCommand((findLatestClangFormat() + " " + std::string{path} + " -i"));
    editor.load();

    return true;
}

bool formatHtmlAndXml(std::filesystem::path path, Editor &editor) {
    if (!isHtml(path) && !isXml(path)) {
        return false;
    }

    if (!hasCommand("tidy")) {
        return false;
    }

    editor.save();
    runCommand("tidy -indent -m --tidy-mark no " +
               std::string{std::filesystem::absolute(path)});
    editor.load();

    return true;
}

bool formatGo(std::filesystem::path path, Editor &editor) {
    if (!isGo(path)) {
        return false;
    }

    if (!hasCommand("gofmt")) {
        return false;
    }

    editor.save();
    runCommand("gofmt -w " + std::string{std::filesystem::absolute(path)});
    editor.load();

    return true;
}

bool formatCMake(std::filesystem::path path, Editor &editor) {
    if (!isCmakeLists(path)) {
        return false;
    }

    if (!hasCommand("cmake-format")) {
        return false;
    }

    editor.save();
    runCommand("cmake-format -i " +
               std::string{std::filesystem::absolute(path)});
    editor.load();

    return true;
}

bool formatPython(std::filesystem::path path, Editor &editor) {
    if (!isPython(path)) {
        return false;
    }

    editor.save();
    /// Use popenstream to parse error messages and annotate on line
    runCommand("black -q " + std::string{std::filesystem::absolute(path)} +
               " 2> " + standardConsoleTtyPipePath().string());
    editor.load();

    return true;
}

bool formatRust(std::filesystem::path path, Editor &editor) {
    if (!isRust(path)) {
        return false;
    }

    editor.save();
    /// Use popenstream to parse error messages and annotate on line
    runCommand("rustfmt " + std::string{std::filesystem::absolute(path)} +
               " 2> " + standardConsoleTtyPipePath().string());
    editor.load();

    return true;
}

} // namespace

bool GenericFormat::format(Editor &editor) {
    auto path = editor.path();
    if (path.empty()) {
        return false;
    }

    constexpr auto functions = std::array{formatClang,
                                          formatHtmlAndXml,
                                          formatGo,
                                          formatCMake,
                                          formatPython,
                                          formatRust};

    for (auto &f : functions) {
        if (f(path, editor)) {
            return true;
        }
    }

    return false;
}
