#pragma once
#include "gitcommitinteraction.h"
#include "core/meditlog.h"
#include "core/os.h"
#include "files/config.h"
#include "files/popenstream.h"
#include "script/interaction.h"
#include "views/interactionhandling.h"
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

void handleGitCommitResponse(std::shared_ptr<IEnvironment> env,
                             const Interaction &interaction,
                             bool isAmend) {

    auto ss = std::istringstream{interaction.text};

    auto message = std::string{};

    for (std::string line; std::getline(ss, line);) {
        if (line.starts_with("#")) {
            continue;
        }
        if (line.starts_with("===")) {
            break;
        }
        message += line + "\n";
    }

    auto files = std::vector<std::string>{};

    if (runCommand("git reset")) {
        return;
    }

    // Parse files
    for (std::string line; std::getline(ss, line);) {
        if (line.starts_with("===")) {
            break;
        }
        if (line.size() <= 3) {
            continue;
        }

        line = line.substr(3);

        if (line.empty()) {
            continue;
        }

        if (runCommand(("git add \"" + line + "\"").c_str())) {
            logError("Failed to add ", line, " to git");
            return;
        }
    }

    auto gitDir = localConfigDirectory("git", true);
    if (!gitDir) {
        return;
    }

    auto messagePath = *gitDir / "commit_message.txt";

    while (!message.empty() && std::isspace(message.back())) {
        message.pop_back();
    }

    while (!message.empty() && std::isspace(message.front())) {
        message.erase(0, 1);
    }

    std::ofstream{messagePath} << message;

    runCommandAndCapture("git commit --file \"" + messagePath.string() + "\" " +
                         (isAmend ? "--amend" : ""));
}

void beginGitCommitImpl(std::shared_ptr<IEnvironment> env, bool isGitAmend) {
    auto stream = POpenStream{"git status --short"};

    auto ss = std::ostringstream{};

    if (isGitAmend) {
        auto stream = POpenStream{"git log -1 --pretty=%B"};
        ss << stream.rdbuf();
        ss << "\n\n# Amend Commit\n\n";
    }

    ss << "\n\n# Write your commit message here and keep the files you want "
          "to\n"
          "# commit below\n";
    ss << "# hint: Press ctrl + return if you want newlines without accepting "
          "\n";
    ss << "# Press return when done\n\n";
    ss << "==============\n\n";
    ss << "============== move files below, or remove this line\n";
    ss << stream.rdbuf();

    auto interaction = Interaction{
        .text = ss.str(),
        .title = "git commit...",
    };

    interaction.begin(env->interactions(), [isGitAmend](auto a, auto b) {
        handleGitCommitResponse(a, b, isGitAmend);
    });
}

} // namespace

void beginGitCommit(std::shared_ptr<IEnvironment> env) {
    beginGitCommitImpl(std::move(env), false);
}

void beginGitCommitAmmend(std::shared_ptr<IEnvironment> env) {
    beginGitCommitImpl(std::move(env), true);
}
