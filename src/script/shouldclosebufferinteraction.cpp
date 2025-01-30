#include "main.h"
#include "script/interaction.h"
#include "script/staticcommandregister.h"
#include "views/editor.h"
#include "views/mainwindow.h"

namespace {

void actuallyClose(std::shared_ptr<IEnvironment> env) {
    auto &buffer = env->editor();
    if (!buffer.closeBuffer()) {
        env->statusMessage(FString{"closing editor..."});
        quitMedit(env->context());
        return;
    }
    env->mainWindow().updateTitle();
}

void handleCloseBufferResponse(std::shared_ptr<IEnvironment> env,
                               const Interaction &i) {}

void beginCloseBufferInteraction(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();
    if (!editor.buffer().isChanged()) {
        actuallyClose(env);
        return;
    }

    auto i = Interaction{
        .text = "",
        .title = "close buffer " + editor.file()->path().filename().string(),
    };

    i.text += "Do you want to close the buffer?\n\n";
    i.text += "path: " + editor.file()->path().string() + "\n\n";
    i.text += " - Yes\n";
    i.text += " - No\n";
    i.text += " - Cancel\n";
}

StaticCommandRegister shouldCloseBufferReg{"close_buffer",
                                           beginCloseBufferInteraction};

} // namespace
