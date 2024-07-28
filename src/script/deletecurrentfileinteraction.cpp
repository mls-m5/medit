
#include "ienvironment.h"
#include "interaction.h"
#include "staticcommandregister.h"
#include "views/editor.h"

namespace {

void handleFileViewResponse(std::shared_ptr<IEnvironment> env,
                            const Interaction &i) {
    auto &editor = env->editor();
    editor.path();
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
