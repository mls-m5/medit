#include "gitdiff.h"
#include "core/coreenvironment.h"
#include "core/meditlog.h"
#include "files/file.h"
#include "files/unsavablefile.h"
#include "syntax/palette.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include "text/linemerge.h"
#include "views/editor.h"
#include <algorithm>
#include <string>
#include <vector>

namespace {

bool hasNoIndex(const Command &command) {
    return std::find(command.arguments.begin(),
                     command.arguments.end(),
                     "--no-index") != command.arguments.end();
}

std::string trimNewline(std::string text) {
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
        text.pop_back();
    }
    return text;
}

} // namespace

Command gitDiffCommand(const GitStatusEntry &entry,
                       const std::filesystem::path &repositoryRoot,
                       bool hasHead) {
    // Independent of the user's color and external diff settings.
    auto arguments =
        std::vector<std::string>{"git", "diff", "--no-color", "--no-ext-diff"};

    const auto exists = std::filesystem::exists(repositoryRoot / entry.path) &&
                        !entry.isDeleted();
    if (entry.index == '?' || (!hasHead && exists)) {
        // Untracked, or nothing to compare with: the whole file is new.
        arguments.insert(arguments.end(),
                         {"--no-index", "--", "/dev/null", entry.path});
    }
    else if (!hasHead) {
        arguments.insert(arguments.end(), {"--cached", "--", entry.path});
    }
    else if (!entry.originalPath.empty()) {
        arguments.insert(arguments.end(),
                         {"-M", "HEAD", "--", entry.originalPath, entry.path});
    }
    else {
        arguments.insert(arguments.end(), {"HEAD", "--", entry.path});
    }
    return Command{std::move(arguments), repositoryRoot};
}

bool isGitDiffSuccess(const Command &command, const CommandResult &result) {
    return result.exitCode == 0 ||
           (result.exitCode == 1 && hasNoIndex(command));
}

FString formatGitDiff(std::string_view diff) {
    auto lines = std::vector<FString>{};
    while (!diff.empty()) {
        const auto end = diff.find('\n');
        const auto line = diff.substr(0, end);
        diff.remove_prefix(end == std::string_view::npos ? diff.size()
                                                         : end + 1);

        auto format = static_cast<FormatType>(Palette::standard);
        if (line.starts_with("diff ") || line.starts_with("index ") ||
            line.starts_with("--- ") || line.starts_with("+++ ") ||
            line.starts_with("new file") || line.starts_with("deleted file") ||
            line.starts_with("similarity") || line.starts_with("rename ") ||
            line.starts_with("old mode") || line.starts_with("new mode")) {
            format = Palette::comment;
        }
        else if (line.starts_with("@@")) {
            format = Palette::statement;
        }
        else if (line.starts_with("+")) {
            format = Palette::string;
        }
        else if (line.starts_with("-")) {
            format = Palette::error;
        }
        lines.emplace_back(std::string{line}, format);
    }
    return FString::join(lines);
}

void showGitDiff(std::shared_ptr<IEnvironment> env,
                 const std::filesystem::path &file) {
    auto &runner = env->core().commandRunner();
    const auto path = std::filesystem::weakly_canonical(file);

    const auto topLevel = runner.run(
        {{"git", "rev-parse", "--show-toplevel"}, path.parent_path()});
    if (topLevel.exitCode) {
        env->statusMessage({"Not in a git repository"});
        return;
    }
    const auto root = std::filesystem::path{trimNewline(topLevel.output)};
    const auto relative = path.lexically_relative(root).generic_string();

    // The whole status, so a rename is reported with both of its paths.
    const auto status = runner.run(
        {{"git", "status", "--porcelain=v1", "-z", "--untracked-files=all"},
         root});
    if (status.exitCode) {
        logError("git status failed: ", status.errors);
        env->statusMessage({"git status failed, see the console"});
        return;
    }
    const auto entries = parseGitStatus(status.output);
    const auto entry =
        std::find_if(entries.begin(),
                     entries.end(),
                     [&relative](const auto &e) { return e.path == relative; });
    if (entry == entries.end()) {
        env->statusMessage(FString{"No changes in " + relative});
        return;
    }

    const auto hasHead =
        runner.run({{"git", "rev-parse", "--verify", "--quiet", "HEAD"}, root})
            .exitCode == 0;
    const auto command = gitDiffCommand(*entry, root, hasHead);
    const auto result = runner.run(command);
    if (!isGitDiffSuccess(command, result)) {
        logError("git diff failed: ", result.errors);
        env->statusMessage({"git diff failed, see the console"});
        return;
    }

    showDiff(env, "git diff " + relative, result.output);
}

void showDiff(std::shared_ptr<IEnvironment> env,
              const std::string &title,
              std::string_view diff) {
    auto buffer = env->core().files().create();
    buffer->assignFile(std::make_unique<UnsavableFile>(title));
    insert(buffer->end(), formatGitDiff(diff));
    // Keep the diff colors; the view is for reading, not saving.
    buffer->isColorsOld(false);
    buffer->isChanged(false);
    env->editor().buffer(buffer);
    env->editor().cursor({*buffer, 0, 0});
}

void showDiffWithDisk(std::shared_ptr<IEnvironment> env) {
    auto &buffer = env->editor().buffer();
    auto *file = buffer.file();
    if (!file) {
        env->statusMessage({"The current buffer has no file"});
        return;
    }
    const auto name = file->path().filename().string();
    const auto disk = file->readDisk();
    if (!disk) {
        env->statusMessage(FString{"Cannot read " + name + " from disk"});
        return;
    }
    const auto diff = unifiedDiff(*disk, fileBytes(buffer));
    if (diff.empty()) {
        env->statusMessage(FString{name + " is the same as on disk"});
        return;
    }
    showDiff(env,
             "disk diff " + name,
             "--- disk/" + name + "\n+++ buffer/" + name + "\n" + diff);
}
