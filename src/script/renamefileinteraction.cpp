#include "renamefileinteraction.h"
#include "core/debugoutput.h"
#include "script/ienvironment.h"
#include "script/interaction.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <memory>
#include <stdexcept>

namespace {

void handleUserResponse(std::shared_ptr<IEnvironment> env,
                        const Interaction &i) {
    debugOutput("got response: ", i.text);

    auto si = SimpleInteraction{};
    si.deserialize(i.text);

    debugOutput("should rename from ", si.at("from"), " to ", si.at("to"));

#warning "continue here"

    throw std::runtime_error{"implement this"};
}

} // namespace

void beginRenameFileInteraction(std::shared_ptr<IEnvironment> env) {
    auto &e = env->editor();
    auto file = e.file();
    if (!file) {
        return;
    }

    auto path = file->path();

    auto si = SimpleInteraction{"rename file",
                                {
                                    {"from", path.string()},
                                    {"to", path.string()},
                                }};

    env->mainWindow().interactions().newInteraction(
        Interaction{si.serialize(), {4, 2}}, handleUserResponse);
}
