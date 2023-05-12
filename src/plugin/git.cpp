#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "views/mainwindow.h"

void registerGitCommands(StandardCommands &standardCommands) {
    standardCommands.namedCommands["git_commit"] =
        [](std::shared_ptr<IEnvironment> env) {
            env->mainWindow().statusMessage(
                "git commit is not implemented yet");
        };
}
