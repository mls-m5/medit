#include "files/popenstream.h"
#include "files/project.h"
#include "gitcommitinteraction.h"
#include "script/ienvironment.h"
#include "script/interaction.h"
#include "script/standardcommands.h"
#include "syntax/palette.h"
#include "text/fstring.h"
#include "views/console.h"
#include "views/editor.h"
#include <istream>
#include <memory>
#include <string>

namespace {

FString universalGitFormat(
    std::istream &stream, std::function<void(FString &)> extraLineFormat = {}) {
    auto ret = std::vector<FString>{};
    for (std::string line; std::getline(stream, line);) {
        auto fstr = FString{line};
        if (line.empty()) {
            continue;
        }
        if (line.front() == '+') {
            // TODO: {2023-11-23} Format + as blue and - as red etc
            fstr.format(Palette::identifier);
        }
        else if (line.front() == '-') {
            fstr.format(Palette::identifier);
        }

        auto str = std::string{line};
        if (str.starts_with("commit ")) {
            fstr.format(Palette::comment, 0, 7);
            fstr.format(Palette::identifier, 7);
        }

        if (str.starts_with("Author: ") || str.starts_with("Date: ")) {
            fstr.format(Palette::comment);
        }
        if (extraLineFormat) {
            extraLineFormat(fstr);
        }
        ret.push_back(std::move(fstr));
    }

    return FString::join(ret);
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

void viewResultAsInteraction(
    std::shared_ptr<IEnvironment> env,
    const std::string &command,
    std::function<void(FString &)> extraLineFormat = {}) {
    auto stream = POpenStream{command};

    auto interaction = Interaction{
        .text = universalGitFormat(stream, extraLineFormat),
        .title = command + "...",
    };

    if (stream.returnCode()) {
        interaction.text += ("\n command failed: " + command);
    }

    interaction.begin(env->interactions(), {});
}

void gitBlame(std::shared_ptr<IEnvironment> env) {
    auto file = env->editor().file();
    if (!file) {
        std::cerr << "could not diff file for editor: editor does not have a "
                     "file associated\n";
        return;
    }

    auto command = "git blame \"" + file->path().string() + "\"";

    auto formatF = [](FString &line) {
        auto f = line.find(' ');

        if (f == FString::npos) {
            return;
        }
        line.format(Palette::comment, 0, f);

        auto f2 = line.find(')', f);
        if (f2 == FString::npos) {
            return;
        }
        line.format(Palette::string, f + 1, f2 + 1);
    };

    viewResultAsInteraction(env, command, formatF);
}

void gitAdd(std::shared_ptr<IEnvironment> env) {
    auto file = env->editor().file();
    if (!file) {
        std::cerr << "could not add file for editor: editor does not have a "
                     "file associated\n";
        return;
    }

    std::cout << "adding current file\n";
    auto command = "git add \"" + file->path().string() + "\"";
    std::cout << command << std::endl;

    auto stream = POpenStream{command};
    std::cout << stream.rdbuf();
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
    standardCommands.namedCommands["git_blame"] = gitBlame;
    standardCommands.namedCommands["git_add"] = gitAdd;
}
