#include "files/file.h"
#include "main.h"
#include "script/interaction.h"
#include "script/saveinteraction.h"
#include "script/staticcommandregister.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <filesystem>
#include <string>
#include <system_error>

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
    auto response = i.lineAtCursor();

    if (response.find("No") != std::string::npos) {
        actuallyClose(env);
        return;
    }

    if (response.find("Yes") != std::string::npos) {
        saveInteraction(env, [env](bool saved) {
            if (saved) {
                actuallyClose(env);
            }
        });
    }
}

void beginCloseBufferInteraction(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();

    // Interaction buffers are temporary and must not trigger another
    // save-confirmation interaction. Close the interaction and restore the
    // buffer that was active before it.
    if (env->mainWindow().interactions().isOperationBuffer(&editor.buffer())) {
        env->mainWindow().interactions().close();
        env->mainWindow().updateTitle();
        return;
    }

    // A loaded file can disappear without any edits in its buffer. The text
    // still needs an explicit save/discard decision before closing.
    bool missingFile = false;
    if (auto *file = dynamic_cast<File *>(editor.file())) {
        std::error_code error;
        missingFile = !std::filesystem::is_regular_file(file->path(), error) ||
                      bool(error);
    }
    if (!editor.buffer().isChanged() && !missingFile) {
        actuallyClose(env);
        return;
    }

    auto i = Interaction{
        .text = "",
        .cursorPosition = {0, 4},
        .title = "close buffer " + editor.path().filename().string(),
    };

    auto path = editor.path().string();
    if (path.empty()) {
        path = "<unnamed>";
    }

    i.text += "Do you want to save the buffer?\n\n";
    i.text += "path: " + path + "\n\n";
    i.text += " - Yes\n";
    i.text += " - No\n";
    i.text += " - Cancel\n";

    env->mainWindow().interactions().newInteraction(i,
                                                    handleCloseBufferResponse);
}

StaticCommandRegister shouldCloseBufferReg{"close_buffer",
                                           beginCloseBufferInteraction};

} // namespace
