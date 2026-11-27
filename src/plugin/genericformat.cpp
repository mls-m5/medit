#include "plugin/genericformat.h"
#include "core/os.h"
#include "files/extensions.h"
#include "views/editor.h"
#include <iostream>
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

} // namespace

std::optional<Command> GenericFormat::formatCommand(
    const std::filesystem::path &path) {
    // Paths are absolute, so they cannot be mistaken for options.
    const auto file = path.string();

    if (isCpp(path) || isC(path) || isJs(path) || isJava(path) ||
        isCSharp(path)) {
        auto clang = findLatestClangFormat();
        if (clang.empty()) {
            // TODO: Print error message
            return {};
        }
        return Command{{clang, file, "-i"}};
    }

    if (isHtml(path) || isXml(path) || isSvg(path)) {
        if (!hasCommand("tidy")) {
            return {};
        }
        auto arguments = std::vector<std::string>{
            "tidy", "-q", "-indent", "-m", "--tidy-mark", "no"};
        if (!isHtml(path)) {
            arguments.push_back("-xml");
        }
        arguments.push_back(file);
        return Command{std::move(arguments)};
    }

    if (isGo(path)) {
        if (!hasCommand("gofmt")) {
            return {};
        }
        return Command{{"gofmt", "-w", file}};
    }

    if (isCmakeLists(path)) {
        if (!hasCommand("cmake-format")) {
            return {};
        }
        return Command{{"cmake-format", "-i", file}};
    }

    // Python and Rust were always handled, also when the tool is missing; the
    // console then shows that it could not be started.
    if (isPython(path)) {
        /// Use popenstream to parse error messages and annotate on line
        return Command{{"black", "-q", file}};
    }

    if (isRust(path)) {
        /// Use popenstream to parse error messages and annotate on line
        return Command{{"rustfmt", file}};
    }

    return {};
}

bool GenericFormat::format(Editor &editor) {
    auto path = editor.path();
    if (path.empty()) {
        return false;
    }

    if (isMsk(path)) {
        // Todo implement this
        return true;
    }

    const auto command = formatCommand(std::filesystem::absolute(path));
    if (!command) {
        return false;
    }

    editor.save();
    // stdout and stderr go to the console, as before.
    const auto result =
        _runner.run(*command, CommandOutput::to(std::cout, std::cerr));
    if (result.exitCode == -1) {
        std::cerr << result.errors << std::endl; // Could not be started.
    }
    editor.load();

    return true;
}
