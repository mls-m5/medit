#include "fileviewinteraction.h"
#include "script/ienvironment.h"
#include "script/interaction.h"
#include "syntax/palette.h"
#include "text/formattype.h"
#include "text/fstring.h"
#include "views/mainwindow.h"
#include <filesystem>
#include <sstream>
#include <string>

namespace {

void internalBeginFileViewInteraction(std::shared_ptr<IEnvironment> env,
                                      std::filesystem::path root);

void handleFileViewResponse(std::shared_ptr<IEnvironment> env,
                            const Interaction &i) {

    auto lineAt = [&](size_t targetNum) {
        auto ss = std::istringstream{i.text};
        auto lineNum = std::size_t{};
        auto result = std::string{};
        for (std::string line; std::getline(ss, line); ++lineNum) {
            if (lineNum == targetNum) {
                result = line;
                break;
            }
        }
        return result;
    };

    auto rootPath = std::filesystem::path(lineAt(1));
    auto result = rootPath / lineAt(i.cursorPosition.y());

    if (result.empty()) {
        env->statusMessage(FString{"no file selected"});
        return;
    }

    if (std::filesystem::is_directory(result)) {
        internalBeginFileViewInteraction(env, result);
        return;
    }

    env->standardCommands().open(env, result, {}, {});
}

FString formatPath(std::filesystem::path path) {

    auto str = FString{};

    if (path.has_parent_path()) {
        str += FString{path.parent_path().string() + "/", Palette::comment};
    }

    str += FString{path.filename().string(), Palette::standard};
    return str;
}

void internalBeginFileViewInteraction(std::shared_ptr<IEnvironment> env,
                                      std::filesystem::path root) {

    auto interaction = Interaction{
        .text = "",
    };

    if (root.empty()) {
        root = env->project().settings().root;
    }

    auto &editor = env->editor();

    size_t currentLine = 1;

    const auto hideHiddenFiles = true;

    if (root.empty()) {
        root = editor.path();
        if (!std::filesystem::is_directory(root) && !root.empty()) {
            root = root.parent_path();
        }
    }

    if (root.empty()) {
        root = std::filesystem::current_path();
    }

    if (std::filesystem::absolute(root).has_parent_path()) {
        interaction.text +=
            std::filesystem::absolute(root).parent_path().string() + "\n";
        ++currentLine;
    }
    interaction.text += std::filesystem::absolute(root).string() + "\n";

    auto addFile = [&](std::filesystem::path file, std::filesystem::path root) {
        if (hideHiddenFiles && file.has_filename() &&
            file.filename().string().starts_with(".")) {
            return;
        }
        auto editorPath = editor.path();
        if (!editorPath.empty() &&
            std::filesystem::equivalent(editorPath, file)) {
            interaction.cursorPosition.y(currentLine);
        }
        auto path = std::filesystem::relative(file, root);

        auto str = formatPath(path);

        interaction.text += str + "\n";

        ++currentLine;
    };

    for (auto &file : env->project().files()) {
        addFile(file, root);
    }

    // If project had no files. Just brows the current directory
    if (env->project().files().empty()) {
        for (auto &it : std::filesystem::directory_iterator{
                 std::filesystem::absolute(root)}) {
            addFile(it.path(), root);
        }
    }

    env->mainWindow().interactions().newInteraction(interaction,
                                                    handleFileViewResponse);
}

} // namespace

void beginFileViewInteraction(std::shared_ptr<IEnvironment> env) {
    internalBeginFileViewInteraction(env, {});
}
