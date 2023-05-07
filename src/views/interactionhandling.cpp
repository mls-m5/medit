#include "interactionhandling.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "keys/event.h"
#include "script/ienvironment.h"
#include "script/interaction.h"
#include "text/cursorrangeops.h"
#include "views/mainwindow.h"
#include <sstream>

void InteractionHandling::newInteraction(const Interaction &i,
                                         InteractionCallback callback) {
    _callback = callback;

    auto ss = std::ostringstream{};
    i.serialize(ss);

    auto editor = _window.currentEditor();
    auto env = _window.env().shared_from_this();

    auto newBuffer = _window.env().core().create(env);
    _operationBuffer = newBuffer;

    editor->buffer(newBuffer);
    editor->bufferView().scroll(0, 0);

    replace(all(*newBuffer), ss.str());
}

bool InteractionHandling::keyPress(std::shared_ptr<IEnvironment> env) {
    auto buffer = _operationBuffer.lock();
    if (!buffer) {
        return false;
    }

    /// This is when the user has edited the response and wants to accept it
    if (env->key().symbol == "\n") {
        auto ss = std::istringstream{content(*buffer)};
        auto interaction = Interaction{};
        interaction.deserialize(ss);

        if (interaction.valid) {
            env->context().guiQueue().addTask(
                [callback = _callback, interaction, env]() {
                    callback(env, interaction);
                });
        }
        else {
            // TODO: Print failing message here
        }

        close();
        return true;
    }
    if (env->key().key == Key::Escape) {
        close();
        return true;
    }
    return false;
}

void InteractionHandling::close() {
    _operationBuffer = {};
    _callback = {};
}
