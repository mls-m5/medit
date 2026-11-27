#pragma once

#include "core/icommandrunner.h"
#include "files/gitstatus.h"
#include "script/ienvironment.h"
#include "text/fstring.h"
#include <filesystem>
#include <memory>
#include <string_view>

/// The command that shows how one file differs from HEAD, staged and unstaged
/// changes together. `entry` is the file's line from git status; `hasHead` is
/// false in a repository without commits.
Command gitDiffCommand(const GitStatusEntry &entry,
                       const std::filesystem::path &repositoryRoot,
                       bool hasHead);

/// `git diff --no-index` exits with 1 when the files differ, which is the
/// normal case; only other exit codes are errors.
bool isGitDiffSuccess(const Command &command, const CommandResult &result);

/// Diff output with colors for added, removed and header lines.
FString formatGitDiff(std::string_view diff);

/// Opens the diff of `file` in the current editor, in a buffer that cannot be
/// saved.
void showGitDiff(std::shared_ptr<IEnvironment> env,
                 const std::filesystem::path &file);
