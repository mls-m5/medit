#pragma once

#include "core/icommandrunner.h"
#include "files/gitstatus.h"
#include "script/ienvironment.h"
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

void beginGitCommit(std::shared_ptr<IEnvironment> env);

void beginGitCommitAmmend(std::shared_ptr<IEnvironment> env);

// The parts below are separate from the interaction so they can be tested.

/// What the user chose in the commit interaction.
struct GitCommitSelection {
    std::string message;
    std::vector<GitStatusEntry> files;
    /// File lines that do not match any line that was shown (edited lines).
    std::vector<std::string> unknownLines;
};

/// The line shown for a file: "XY path", or "XY path <- original" for renames
/// and copies. Newlines in names are shown as "\n".
std::string formatGitCommitFileLine(const GitStatusEntry &file);

/// The text the user edits. `previousMessage` is used when amending.
std::string gitCommitTemplate(std::string_view previousMessage,
                              bool isAmend,
                              const std::vector<GitStatusEntry> &files);

/// Reads the message and the selected files from the edited template.
GitCommitSelection parseGitCommitResponse(
    std::string_view text, const std::vector<GitStatusEntry> &files);

/// Unstages everything, stages exactly the selected files and commits them
/// with the message. Returns the first failing step, otherwise the commit.
/// Nothing is changed when no files are selected.
CommandResult commitGitSelection(ICommandRunner &runner,
                                 const std::filesystem::path &repositoryRoot,
                                 const GitCommitSelection &selection,
                                 bool isAmend,
                                 const CommandOutput &commitOutput = {});
