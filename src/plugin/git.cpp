#include "core/coreenvironment.h"
#include "core/icommandrunner.h"
#include "files/project.h"
#include "gitcommitinteraction.h"
#include "script/ienvironment.h"
#include "script/interaction.h"
#include "script/standardcommands.h"
#include "syntax/palette.h"
#include "text/fstring.h"
#include "views/console.h"
#include "views/editor.h"
#include <iostream>
#include <istream>
#include <memory>
#include <sstream>
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
        .command = Command{{"git", "push"}, root},
        .buffer = &env->console().buffer(),
    };

    console.run(env);
}

void viewResultAsInteraction(
    std::shared_ptr<IEnvironment> env,
    const Command &command,
    std::function<void(FString &)> extraLineFormat = {}) {
    auto result = env->core().commandRunner().run(command);
    auto stream = std::istringstream{result.output};

    auto interaction = Interaction{
        .text = universalGitFormat(stream, extraLineFormat),
        .title = command.description() + "...",
    };

    if (result.exitCode) {
        interaction.text += ("\n command failed: " + command.description());
        if (!result.errors.empty()) {
            interaction.text += "\n" + result.errors;
        }
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

    const auto path = file->path();
    const auto command =
        Command{{"git", "blame", "--", path.string()}, path.parent_path()};

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
    const auto path = file->path();
    const auto command =
        Command{{"git", "add", "--", path.string()}, path.parent_path()};
    std::cout << command.description() << std::endl;

    env->core().commandRunner().run(command,
                                    CommandOutput::to(std::cout, std::cerr));
}

} // namespace

void registerGitCommands(StandardCommands &sc) {
    auto wrapInteraction = [](std::vector<std::string> arguments)
        -> std::function<void(std::shared_ptr<IEnvironment>)> {
        arguments.insert(arguments.begin(), "git");
        return [command = Command{std::move(arguments)}](auto env) {
            viewResultAsInteraction(env, command);
        };
    };

    sc.addCommand("git_status", wrapInteraction({"status"}), nullptr);
    sc.addCommand("git_diff", wrapInteraction({"diff"}), nullptr);
    sc.addCommand(
        "git_diff_cached", wrapInteraction({"diff", "--cached"}), nullptr);
    sc.addCommand("git_push", gitPush, nullptr);
    sc.addCommand("git_log", wrapInteraction({"log"}), nullptr);
    sc.addCommand("git_commit", beginGitCommit, nullptr);
    sc.addCommand("git_commit_amend", beginGitCommitAmmend, nullptr);
    sc.addCommand("git_blame", gitBlame, nullptr);
    sc.addCommand("git_add", gitAdd, nullptr);
}
