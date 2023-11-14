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

void handleFileViewResponse(std::shared_ptr<IEnvironment> env,
                            const Interaction &i) {

    auto ss = std::istringstream{i.text};
    auto lineNum = std::size_t{};
    auto result = std::string{};
    for (std::string line; std::getline(ss, line); ++lineNum) {
        if (lineNum == i.cursorPosition.y()) {
            result = line;
            break;
        }
    }
    if (result.empty()) {
        env->statusMessage(FString{"no file selected"});
        return;
    }

    env->standardCommands().open(env, result, {}, {});
}
} // namespace

void beginFileViewInteraction(std::shared_ptr<IEnvironment> env) {
    auto interaction = Interaction{
        .text = "",
    };

    auto root = env->project().settings().root;

    auto &editor = env->editor();

    size_t currentLine = 0;

    for (auto &file : env->project().files()) {
        if (editor.file() &&
            std::filesystem::equivalent(editor.file()->path(), file)) {
            interaction.cursorPosition.y(currentLine);
        }
        auto path = std::filesystem::relative(file, root);

        auto str = FString{};

        if (path.has_parent_path()) {
            str += FString{path.parent_path().string() + "/", Palette::comment};
        }

        str += FString{path.filename().string(), Palette::standard};

        interaction.text += str + "\n";

        ++currentLine;
    }

    env->mainWindow().interactions().newInteraction(interaction,
                                                    handleFileViewResponse);
}
