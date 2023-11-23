#include "core/os.h"
#include "files/popenstream.h"
#include "files/project.h"
#include "gitcommitinteraction.h"
#include "script/ienvironment.h"
#include "script/interaction.h"
#include "script/standardcommands.h"
#include "syntax/palette.h"
#include "views/console.h"
#include "views/editor.h"
#include <memory>
#include <string>
#include <string_view>

namespace {

void gitPush(std::shared_ptr<IEnvironment> env) {
    auto root = env->project().settings().root;
    auto console = Console{
        .openingMessage = FString{"git push", Palette::comment},
        .command = "git -C \"" + root.string() + "\" push",
        .buffer = &env->console().buffer(),
    };

    console.run(env);
}

void viewResultAsInteraction(std::shared_ptr<IEnvironment> env,
                             const std::string &command) {
    // TODO: {2023-11-23} Format + as blue and - as red etc
    auto stream = POpenStream{command};

    auto interaction = Interaction{
        .text = stream.readToString(),
        .title = command + "...",
    };

    interaction.begin(env->interactions(), {});
}

} // namespace

void registerGitCommands(StandardCommands &standardCommands) {
    //    auto git = std::string{"git -c color.status=always "};
    auto git = std::string{"git "};

    auto wrapInteraction = [](std::string command)
        -> std::function<void(std::shared_ptr<IEnvironment>)> {
        return [command](auto env) { viewResultAsInteraction(env, command); };
    };

    standardCommands.namedCommands["git_status"] =
        wrapInteraction(git + " status");
    standardCommands.namedCommands["git_diff"] = wrapInteraction(git + " diff");
    standardCommands.namedCommands["git_diff_cached"] =
        wrapInteraction(git + " diff --cached");
    standardCommands.namedCommands["git_push"] = gitPush;
    standardCommands.namedCommands["git_log"] = wrapInteraction(git + " log");
    standardCommands.namedCommands["git_commit"] = beginGitCommit;
    standardCommands.namedCommands["git_commit_amend"] = beginGitCommitAmmend;
}
