#include "saveinteraction.h"
#include "interaction.h"
#include "script/ienvironment.h"
#include "views/mainwindow.h"
#include <filesystem>
#include <fstream>
#include <memory>

namespace {

void handleUserFileNameResponse(std::shared_ptr<IEnvironment> env,
                                const Interaction &i) {
    auto si = SimpleInteraction{};

    si.deserialize(i.text);

    auto path = si.at("file");

    if (path.empty()) {
        return;
    }

    if (std::filesystem::exists(path)) {
        auto si = SimpleInteraction{
            "File exists, do you want to save to another file?",
            {
                {"file", std::string{path}},
            }};

        env->mainWindow().interactions().newInteraction(
            i, handleUserFileNameResponse);
        return;
    }

    env->editor().buffer().saveAs(path);
}

} // namespace

void saveInteraction(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    if (e.file()) {
        e.save();
        return;
    }

    if (env->mainWindow().interactions().isOperationBuffer(&e.buffer())) {
        return;
    }

    auto i = Interaction{};
    i.cursorPosition = {1, 100000};

    auto si = SimpleInteraction{
        "save?",
        {
            {"file", env->project().settings().root / "unnamed.txt"},
        }};

    i.text = si.serialize();

    env->mainWindow().interactions().newInteraction(i,
                                                    handleUserFileNameResponse);
}
