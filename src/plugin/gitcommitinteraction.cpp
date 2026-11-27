#pragma once
#include "gitcommitinteraction.h"
#include "core/coreenvironment.h"
#include "core/icommandrunner.h"
#include "core/meditlog.h"
#include "files/project.h"
#include "script/interaction.h"
#include "views/interactionhandling.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr auto selectedSeparator = std::string_view{"=============="};
constexpr auto unselectedSeparator = std::string_view{
    "============== move files up, or remove this line to select files"};

std::string displayPath(std::string_view path) {
    auto ret = std::string{};
    for (const auto c : path) {
        if (c == '\n') {
            ret += "\\n";
        }
        else {
            ret += c;
        }
    }
    return ret;
}

std::string trim(std::string text) {
    while (!text.empty() &&
           std::isspace(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }
    auto begin = size_t{0};
    while (begin < text.size() &&
           std::isspace(static_cast<unsigned char>(text[begin]))) {
        ++begin;
    }
    return text.substr(begin);
}

bool isBlank(std::string_view line) {
    return std::all_of(line.begin(), line.end(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c));
    });
}

/// Logs the errors of a failed step and returns true if it failed.
bool failed(const CommandResult &result, std::string_view what) {
    if (!result.exitCode) {
        return false;
    }
    logError(what, " failed: ", result.errors);
    return true;
}

void handleGitCommitResponse(std::shared_ptr<IEnvironment> env,
                             const Interaction &interaction,
                             bool isAmend,
                             const std::filesystem::path &repositoryRoot,
                             const std::vector<GitStatusEntry> &files) {
    const auto selection =
        parseGitCommitResponse(std::string{interaction.text}, files);

    if (!selection.unknownLines.empty()) {
        for (const auto &line : selection.unknownLines) {
            logError("git commit: not a line from git status: ", line);
        }
        env->statusMessage(
            {"Edited file lines in git commit. Nothing committed."});
        return;
    }

    // Checked before anything is unstaged.
    if (selection.files.empty()) {
        env->statusMessage({"No files selected for git commit. Abort..."});
        return;
    }

    const auto result =
        commitGitSelection(env->core().commandRunner(),
                           repositoryRoot,
                           selection,
                           isAmend,
                           CommandOutput::to(std::cout, std::cerr));

    if (failed(result, "git commit")) {
        env->statusMessage({"git commit failed, see the console"});
        return;
    }
    env->statusMessage({"files commited to git"});
}

void beginGitCommitImpl(std::shared_ptr<IEnvironment> env, bool isGitAmend) {
    auto &runner = env->core().commandRunner();
    const auto projectRoot = env->project().settings().root;

    const auto topLevel =
        runner.run({{"git", "rev-parse", "--show-toplevel"}, projectRoot});
    if (failed(topLevel, "git rev-parse")) {
        env->statusMessage({"Not in a git repository"});
        return;
    }
    const auto repositoryRoot = std::filesystem::path{trim(topLevel.output)};

    const auto status =
        runner.run({{"git", "status", "--porcelain=v1", "-z"}, repositoryRoot});
    if (failed(status, "git status")) {
        env->statusMessage({"git status failed, see the console"});
        return;
    }
    auto files = parseGitStatus(status.output);

    auto previousMessage = std::string{};
    if (isGitAmend) {
        previousMessage =
            runner.run({{"git", "log", "-1", "--pretty=%B"}, repositoryRoot})
                .output;
    }

    auto interaction = Interaction{
        .text = gitCommitTemplate(previousMessage, isGitAmend, files),
        .title = "git commit...",
    };

    interaction.begin(
        env->interactions(),
        [isGitAmend, repositoryRoot, files = std::move(files)](
            auto env, const auto &interaction) {
            handleGitCommitResponse(
                env, interaction, isGitAmend, repositoryRoot, files);
        });
}

} // namespace

void beginGitCommit(std::shared_ptr<IEnvironment> env) {
    beginGitCommitImpl(std::move(env), false);
}

void beginGitCommitAmmend(std::shared_ptr<IEnvironment> env) {
    beginGitCommitImpl(std::move(env), true);
}

std::string formatGitCommitFileLine(const GitStatusEntry &file) {
    auto line = std::string{file.index, file.worktree, ' '};
    line += displayPath(file.path);
    if (!file.originalPath.empty()) {
        line += " <- " + displayPath(file.originalPath);
    }
    return line;
}

std::string gitCommitTemplate(std::string_view previousMessage,
                              bool isAmend,
                              const std::vector<GitStatusEntry> &files) {
    auto ss = std::ostringstream{};

    if (isAmend) {
        ss << previousMessage;
        ss << "\n\n# Amend Commit\n\n";
    }

    ss << "\n\n# Write your commit message here and keep the files you want "
          "to\n"
          "# commit below\n";
    ss << "# hint: Press ctrl + return if you want newlines without accepting "
          "\n";
    ss << "# Press return when done\n\n";
    ss << selectedSeparator << "\n\n";
    ss << unselectedSeparator << "\n";
    for (const auto &file : files) {
        ss << formatGitCommitFileLine(file) << "\n";
    }
    return ss.str();
}

GitCommitSelection parseGitCommitResponse(
    std::string_view text, const std::vector<GitStatusEntry> &files) {
    auto selection = GitCommitSelection{};
    auto ss = std::istringstream{std::string{text}};

    for (std::string line; std::getline(ss, line);) {
        if (line.starts_with("#")) {
            continue;
        }
        if (line.starts_with("===")) {
            break;
        }
        selection.message += line + "\n";
    }
    selection.message = trim(std::move(selection.message));

    // Files between the two separators are selected; all files if the second
    // separator was removed.
    for (std::string line; std::getline(ss, line);) {
        if (line.starts_with("===")) {
            break;
        }
        if (isBlank(line)) {
            continue;
        }
        const auto found =
            std::find_if(files.begin(), files.end(), [&line](const auto &f) {
                return formatGitCommitFileLine(f) == line;
            });
        if (found == files.end()) {
            selection.unknownLines.push_back(line);
        }
        else if (std::find(selection.files.begin(),
                           selection.files.end(),
                           *found) == selection.files.end()) {
            selection.files.push_back(*found);
        }
    }

    return selection;
}

CommandResult commitGitSelection(ICommandRunner &runner,
                                 const std::filesystem::path &repositoryRoot,
                                 const GitCommitSelection &selection,
                                 bool isAmend,
                                 const CommandOutput &commitOutput) {
    if (selection.files.empty()) {
        auto result = CommandResult{};
        result.errors = "no files selected";
        return result;
    }

    auto add = std::vector<std::string>{"git", "add", "--"};
    auto remove = std::vector<std::string>{
        "git", "rm", "--cached", "--quiet", "--ignore-unmatch", "--"};
    const auto removeArguments = remove.size();
    for (const auto &file : selection.files) {
        (file.isDeleted() ? remove : add).push_back(file.path);
        if (file.index == 'R' || file.worktree == 'R') {
            remove.push_back(file.originalPath); // A copy keeps its source.
        }
    }

    // Start from HEAD, so that only the selected files are committed.
    if (auto result = runner.run({{"git", "reset", "--quiet"}, repositoryRoot});
        result.exitCode) {
        return result;
    }
    if (add.size() > 3) {
        if (auto result = runner.run({add, repositoryRoot}); result.exitCode) {
            return result;
        }
    }
    if (remove.size() > removeArguments) {
        if (auto result = runner.run({remove, repositoryRoot});
            result.exitCode) {
            return result;
        }
    }

    auto commit = std::vector<std::string>{"git", "commit", "--file=-"};
    if (isAmend) {
        commit.push_back("--amend");
    }
    return runner.run({commit, repositoryRoot, selection.message},
                      commitOutput);
}
