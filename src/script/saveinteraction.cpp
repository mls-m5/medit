#include "saveinteraction.h"
#include "core/coreenvironment.h"
#include "files/project.h"
#include "interaction.h"
#include "script/ienvironment.h"
#include "script/simpleinteraction.h"
#include "views/mainwindow.h"
#include <filesystem>
#include <memory>
#include <stdexcept>

namespace {

void handleUserFileNameResponse(std::shared_ptr<IEnvironment> env,
                                const Interaction &i,
                                std::shared_ptr<Buffer> buffer) {
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
            ni, [buffer](auto &&a, auto &&b) {
                handleUserFileNameResponse(a, b, buffer);
            });
        return;
    }

    try {
        if (!env->core().files().save(*buffer, path)) {
            // Just use single handling since errors was already handled
            throw std::runtime_error{"file is not saveable"};
        }
        env->mainWindow().statusMessage(FString{"saved to "} +
                                        buffer->file()->path().string());
    }
    catch (std::runtime_error &e) {
        env->statusMessage(FString{"Could not save file "} + FString{path} +
                           ": " + e.what());
    }
}

} // namespace

void saveInteraction(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    if (e.file()) {
        if (!e.save()) {
            env->mainWindow().statusMessage(FString{"Could not save "} +
                                            e.file()->path().string());
            return;
        }
        env->mainWindow().statusMessage(FString{"Saved to "} +
                                        e.file()->path().string());
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

    auto buffer = e.buffer().shared_from_this();

    env->mainWindow().interactions().newInteraction(
        i, [buffer](auto &&a, auto &&b) {
            handleUserFileNameResponse(a, b, buffer);
        });
}
