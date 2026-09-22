#include "main.h"
#include "script/interaction.h"
#include "script/staticcommandregister.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <string>

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
                               const Interaction &i) {
    if (i.lineAtCursor().find("Yes") == std::string::npos) {
        return;
    }

    actuallyClose(env);
}

void beginCloseBufferInteraction(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();
    if (!editor.buffer().isChanged()) {
        actuallyClose(env);
        return;
    }

    auto i = Interaction{
        .text = "",
        .cursorPosition = {0, 4},
        .title = "close buffer " + editor.path().filename().string(),
    };

    i.text += "Do you want to close the buffer?\n\n";
    i.text += "path: " + editor.path().string() + "\n\n";
    i.text += " - Yes\n";
    i.text += " - No\n";
    i.text += " - Cancel\n";

    env->mainWindow().interactions().newInteraction(i,
                                                    handleCloseBufferResponse);
}

StaticCommandRegister shouldCloseBufferReg{"close_buffer",
                                           beginCloseBufferInteraction};

} // namespace
