#include "interactionhandling.h"
#include "core/context.h"
#include "core/ijobqueue.h"
#include "keys/event.h"
#include "script/ienvironment.h"
#include "script/interaction.h"
#include "text/cursorrangeops.h"
#include "text/fstring.h"
#include "views/mainwindow.h"
#include <memory>
#include <sstream>

void InteractionHandling::newInteraction(const Interaction &i,
                                         InteractionCallback callback) {
    _callback = callback;

    // TODO: Open in some special window
    auto editor = _window.currentEditor();
    if (!editor) {
        return;
    }

    auto env = _window.env().shared_from_this();

    // We do not create with the core create function because we do not want
    // to add it to the tracked buffers
    auto newBuffer = std::make_shared<Buffer>();
    _operationBuffer = newBuffer;

    editor->buffer(newBuffer);

    _editor = editor->weak_from_this();

    replace(all(*newBuffer), i.text);
    editor->cursor(i.cursorPosition);
}

bool InteractionHandling::keyPress(std::shared_ptr<IEnvironment> env) {
    auto buffer = _operationBuffer.lock();
    if (!buffer) {
        return false;
    }

    /// This is when the user has edited the response and wants to accept it
    if (env->key().symbol == "\n") {
        //        auto ss = std::istringstream{content(*buffer)};
        auto interaction = Interaction{.text = content(*buffer)};
        //        interaction.deserialize(ss);

        //        if (interaction.valid) {
        env->context().guiQueue().addTask(
            [callback = _callback, interaction, env]() {
                callback(env, interaction);
            });
        //        }
        //        else {
        //            // TODO: Print failing message here
        //        }

        close();
        return true;
    }
    //    if (env->key().key == Key::Escape) {
    //        close(); Use ctrl+w instead
    //        return true;
    //    }
    return false;
}

void InteractionHandling::close() {
    _operationBuffer = {};
    _callback = {};
    if (auto editor = _editor.lock()) {
        editor->closeBuffer();
        editor.reset();
    }
}
