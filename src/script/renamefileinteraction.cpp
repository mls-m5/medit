#include "renamefileinteraction.h"
#include "core/coreenvironment.h"
#include "core/debugoutput.h"
#include "script/ienvironment.h"
#include "script/interaction.h"
#include "text/fstring.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <memory>
#include <stdexcept>

namespace {

void handleUserResponse(std::shared_ptr<IEnvironment> env,
                        const Interaction &i) {
    auto si = SimpleInteraction{};
    si.deserialize(i.text);

    auto path = si.at("from");

    auto buffer = env->core().files().find(path);

    if (!buffer) {
        debugOutput(
            "failed to rename from ", si.at("from"), " to ", si.at("to"));
    }

    auto file = buffer->file();

    if (!file) {
        debugOutput(
            "failed to rename from ", si.at("from"), " to ", si.at("to"));
    }

    if (env->core().files().rename(si.at("from"), si.at("to"))) {
        debugOutput(
            "failed to rename from ", si.at("from"), " to ", si.at("to"));
    }
}

} // namespace

void beginRenameFileInteraction(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    auto file = e.file();
    if (!file) {
        return;
    }

    auto path = file->path();

    auto pathStr = FString{path.string()};

    auto si = SimpleInteraction{"rename file",
                                {
                                    {"from", pathStr},
                                    {"to", pathStr},
                                }};

    env->mainWindow().interactions().newInteraction(
        Interaction{si.serialize(), {3 + pathStr.size(), 2}},
        handleUserResponse);
}
