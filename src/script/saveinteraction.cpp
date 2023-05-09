#include "saveinteraction.h"
#include "core/coreenvironment.h"
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

    if (si.at("overwrite") != "yes" && std::filesystem::exists(path)) {
        auto si = SimpleInteraction{
            "File exists, submit again if you want to save to this file?",
            {
                {"file", std::string{path}},
                {"overwrite", "yes"},
            }};

        auto ni = Interaction{si.serialize(), {100000, 1}};

        env->mainWindow().interactions().newInteraction(
            ni, handleUserFileNameResponse);
        return;
    }

    env->core().save(env->editor().buffer(), path);
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
    i.cursorPosition = {100000, 1};

    auto si = SimpleInteraction{
        "save?",
        {
            {"file", env->project().settings().root / "unnamed.txt"},
            {"overwrite", "ask"},
        }};

    i.text = si.serialize();

    env->mainWindow().interactions().newInteraction(i,
                                                    handleUserFileNameResponse);
}
