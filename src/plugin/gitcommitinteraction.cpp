#pragma once
#include "gitcommitinteraction.h"
#include "core/meditlog.h"
#include "core/os.h"
#include "files/config.h"
#include "files/popenstream.h"
#include "script/interaction.h"
#include "views/interactionhandling.h"
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

void handleGitCommitResponse(std::shared_ptr<IEnvironment> env,
                             const Interaction &interaction) {

    auto ss = std::istringstream{interaction.text};

    auto message = std::string{};

    for (std::string line; std::getline(ss, line);) {
        if (line.starts_with("#")) {
            continue;
        }
        if (line.starts_with("====")) {
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

    std::ofstream{messagePath} << message;

    runCommand("git commit --file \"" + messagePath.string() + "\"");
}

} // namespace

void beginGitCommit(std::shared_ptr<IEnvironment> env) {
    auto stream = POpenStream{"git status --short"};

    auto ss = std::ostringstream{};
    ss << "\n\n# Write your commit message here and keep the files you want "
          "to\n "
          "commit below\n";
    ss << "# hint: Press ctrl + return if you want newlines without accepting "
          "\n";
    ss << "# Press return when done\n\n";
    ss << "==============\n";
    ss << stream.rdbuf();

    auto interaction = Interaction{
        .text = ss.str(),
        .title = "git commit...",
    };

    interaction.begin(env->interactions(), handleGitCommitResponse);
}
