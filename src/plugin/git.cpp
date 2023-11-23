#include "files/project.h"
#include "gitcommitinteraction.h"
#include "script/ienvironment.h"
#include "script/standardcommands.h"
#include "syntax/palette.h"
#include "views/console.h"
#include "views/editor.h"
#include <memory>

namespace {

void gitStatus(std::shared_ptr<IEnvironment> env) {
    auto root = env->project().settings().root;
    auto console = Console{
        .openingMessage = FString{"git status", Palette::comment},
        .command = "git -C \"" + root.string() + "\" status",
        .buffer = &env->console().buffer(),
    };

    console.run(env);
}

void gitPush(std::shared_ptr<IEnvironment> env) {
    auto root = env->project().settings().root;
    auto console = Console{
        .openingMessage = FString{"git push", Palette::comment},
        .command = "git -C \"" + root.string() + "\" push",
        .buffer = &env->console().buffer(),
    };

    console.run(env);
}

} // namespace

void registerGitCommands(StandardCommands &standardCommands) {
    standardCommands.namedCommands["git_status"] = gitStatus;
    standardCommands.namedCommands["git_push"] = gitPush;
    standardCommands.namedCommands["git_commit"] = beginGitCommit;
    standardCommands.namedCommands["git_commit_amend"] = beginGitCommitAmmend;
}
