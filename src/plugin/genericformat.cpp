#include "plugin/genericformat.h"
#include "core/os.h"
#include "files/extensions.h"
#include "files/ifile.h"
#include "views/editor.h"

namespace {

std::string findLatestClangFormat() {
    for (int i = 30; i > 5; --i) {
        auto command = "clang-format-" + std::to_string(i);
        hasCommand(command);
        return command;
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

} // namespace

bool GenericFormat::format(Editor &editor) {
    auto path = editor.path();
    if (path.empty()) {
        return false;
    }

    if (formatClang(path, editor)) {
        return true;
    }
    if (formatHtmlAndXml(path, editor)) {
        return true;
    }
    if (formatGo(path, editor)) {
        return true;
    }

    return false;
}
