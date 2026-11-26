#include "closewindowinteraction.h"
#include "core/coreenvironment.h"
#include "core/files.h"
#include "files/file.h"
#include "files/unsavablefile.h"
#include "main.h"
#include "script/interaction.h"
#include "script/saveinteraction.h"
#include "script/staticcommandregister.h"
#include "views/editor.h"
#include "views/mainwindow.h"
#include <filesystem>
#include <functional>
#include <string>
#include <system_error>
#include <unordered_set>
#include <vector>

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

struct CloseWindowState {
    std::shared_ptr<IEnvironment> env;
    std::vector<std::shared_ptr<Buffer>> buffers;
    size_t index = 0;
    std::function<void(bool)> completed;
};

void promptNextBuffer(const std::shared_ptr<CloseWindowState> &state) {
    if (state->index == state->buffers.size()) {
        state->completed(true);
        return;
    }
    const auto buffer = state->buffers[state->index];
    auto path = buffer->path().string();
    if (path.empty())
        path = "<unnamed>";
    auto prompt = Interaction{
        .text = "Do you want to save the buffer?\n\npath: " + path +
                "\n\n - Yes\n - No\n - Cancel\n",
        .cursorPosition = {0, 4},
        .title = "close buffer " + buffer->path().filename().string(),
    };
    state->env->mainWindow().interactions().newInteraction(
        prompt,
        [state, buffer](auto env, const Interaction &response) {
            const auto answer = response.lineAtCursor();
            if (answer.find("No") != std::string::npos) {
                ++state->index;
                promptNextBuffer(state);
            }
            else if (answer.find("Yes") != std::string::npos) {
                saveInteraction(env, buffer, [state](bool saved) {
                    if (!saved) {
                        state->completed(false);
                        return;
                    }
                    ++state->index;
                    promptNextBuffer(state);
                });
            }
            else {
                state->completed(false);
            }
        },
        [state] { state->completed(false); });
}

} // namespace

void beginCloseWindowInteraction(std::shared_ptr<IEnvironment> env,
                                 std::function<void(bool)> completed) {
    auto &window = env->mainWindow();
    if (window.interactions().active()) {
        env->statusMessage(FString{"Finish the current interaction first"});
        completed(false);
        return;
    }
    auto state = std::make_shared<CloseWindowState>();
    state->env = std::move(env);
    state->completed = std::move(completed);
    std::unordered_set<Buffer *> seen;
    for (auto &buffer : state->env->core().files().buffers()) {
        if (!seen.insert(buffer.get()).second)
            continue;
        // Console and interaction buffers are temporary. Do not ask to save
        // them, but retain unnamed editor buffers with unsaved text.
        if (dynamic_cast<UnsavableFile *>(buffer->file()))
            continue;
        bool missingFile = false;
        if (auto *file = dynamic_cast<File *>(buffer->file())) {
            std::error_code error;
            missingFile =
                !std::filesystem::is_regular_file(file->path(), error) ||
                bool(error);
        }
        if (buffer->isChanged() || missingFile)
            state->buffers.push_back(buffer);
    }
    if (!state->buffers.empty())
        state->env->mainWindow().focusEditor();
    promptNextBuffer(state);
}
