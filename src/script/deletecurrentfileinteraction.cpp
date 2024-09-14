
#include "core/coreenvironment.h"
#include "core/files.h"
#include "files/project.h"
#include "ienvironment.h"
#include "interaction.h"
#include "staticcommandregister.h"
#include "views/editor.h"

namespace {

void handleDeleteFileResponseResponse(std::shared_ptr<IEnvironment> env,
                                      const Interaction &i) {
    auto path = std::filesystem::path(i.lineAt(1));
    if (i.lineAtCursor() != "yes") {
        return;
    }

    env->core().files().deleteFile(path);
}

void beginDeleteCurrentFileInteraction(std::shared_ptr<IEnvironment> env) {
    auto &editor = env->editor();
    auto interaction = Interaction{
        .text = "Do you really want to remove the file\n" +
                editor.path().string() +
                "yes\n"
                "no\n",
        .cursorPosition{0, 2},
        .title = "delete file...",
    };
}

} // namespace

StaticCommandRegister deleteCurrentFileReg{"delete_current_file",
                                           beginDeleteCurrentFileInteraction};
