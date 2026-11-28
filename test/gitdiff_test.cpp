#include "core/icommandrunner.h"
#include "files/gitstatus.h"
#include "mls-unit-test/unittest.h"
#include "script/gitdiff.h"
#include "syntax/palette.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace git_diff_test {

struct Repository {
    std::filesystem::path path;
    std::unique_ptr<ICommandRunner> runner = createCommandRunner();

    Repository() {
        auto pattern =
            (std::filesystem::temp_directory_path() / "medit-diff-test-XXXXXX")
                .string();
        const auto *created = mkdtemp(pattern.data());
        if (!created) {
            throw std::runtime_error("Cannot create temporary directory");
        }
        path = std::filesystem::canonical(created);
        git({"init", "-q"});
        git({"config", "user.name", "Test"});
        git({"config", "user.email", "test@example.com"});
        git({"config", "commit.gpgsign", "false"});
    }

    ~Repository() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }

    std::string git(std::vector<std::string> arguments) {
        arguments.insert(arguments.begin(), "git");
        const auto result = runner->run({arguments, path});
        if (result.exitCode) {
            throw std::runtime_error("git failed: " + result.errors);
        }
        return result.output;
    }

    void write(const std::string &name, const std::string &content) {
        std::ofstream{path / name} << content;
    }

    GitStatusEntry status(const std::string &name) {
        for (auto &entry : parseGitStatus(git({"status",
                                               "--porcelain=v1",
                                               "-z",
                                               "--untracked-files=all"}))) {
            if (entry.path == name) {
                return entry;
            }
        }
        throw std::runtime_error("no status for " + name);
    }

    /// Runs the diff command for `name` and returns its output.
    std::string diff(const std::string &name, bool hasHead = true) {
        const auto command = gitDiffCommand(status(name), path, hasHead);
        const auto result = runner->run(command);
        if (!isGitDiffSuccess(command, result)) {
            throw std::runtime_error("diff failed: " + result.errors);
        }
        return result.output;
    }
};

CommandResult exited(int exitCode) {
    auto result = CommandResult{};
    result.exitCode = exitCode;
    return result;
}

bool contains(const std::string &text, const std::string &part) {
    return text.find(part) != std::string::npos;
}

} // namespace git_diff_test

TEST_SUIT_BEGIN(GitDiff)

TEST_CASE("the diff command depends on the file's status") {
    const auto root = std::filesystem::path{"/repo"};
    auto modified = GitStatusEntry{};
    modified.index = 'M';
    modified.path = "a b.txt";
    ASSERT_EQ(gitDiffCommand(modified, root, true).description(),
              "git diff --no-color --no-ext-diff HEAD -- a b.txt");
    ASSERT_EQ(gitDiffCommand(modified, root, true).workingDirectory, root);

    auto renamed = modified;
    renamed.index = 'R';
    renamed.originalPath = "old.txt";
    ASSERT_EQ(gitDiffCommand(renamed, root, true).description(),
              "git diff --no-color --no-ext-diff -M HEAD -- old.txt a b.txt");

    auto untracked = modified;
    untracked.index = '?';
    untracked.worktree = '?';
    const auto untrackedCommand = gitDiffCommand(untracked, root, true);
    ASSERT_EQ(
        untrackedCommand.description(),
        "git diff --no-color --no-ext-diff --no-index -- /dev/null a b.txt");

    // --no-index reports differences with exit code 1.
    EXPECT_TRUE(isGitDiffSuccess(untrackedCommand, git_diff_test::exited(1)));
    EXPECT_FALSE(
        isGitDiffSuccess(untrackedCommand, git_diff_test::exited(128)));
    EXPECT_FALSE(isGitDiffSuccess(gitDiffCommand(modified, root, true),
                                  git_diff_test::exited(1)));
}

TEST_CASE("diffs of modified, staged, untracked, renamed and deleted files") {
    git_diff_test::Repository repo;
    const auto odd = std::string{"say \"hi\" $HOME `x`.txt"};
    repo.write(odd, "one\ntwo\n");
    repo.write("old name.txt", "keep\nthis\n");
    repo.write("gone.txt", "bye\n");
    repo.git({"add", "--all"});
    repo.git({"commit", "-q", "-m", "base"});

    repo.write(odd, "one\nTWO\n"); // Unstaged change.
    repo.git({"mv", "old name.txt", "new name.txt"});
    repo.write("staged.txt", "first\n");
    repo.git({"add", "staged.txt"});
    repo.write("staged.txt", "first\nsecond\n"); // Staged and unstaged.
    repo.write("untracked.txt", "fresh\n");
    std::filesystem::remove(repo.path / "gone.txt");

    const auto modified = repo.diff(odd);
    EXPECT_TRUE(git_diff_test::contains(modified, "-two\n+TWO\n"));

    const auto staged = repo.diff("staged.txt");
    EXPECT_TRUE(git_diff_test::contains(staged, "+first\n+second\n"));

    const auto untracked = repo.diff("untracked.txt");
    EXPECT_TRUE(git_diff_test::contains(untracked, "+fresh\n"));

    const auto renamed = repo.diff("new name.txt");
    EXPECT_TRUE(git_diff_test::contains(renamed, "rename from old name.txt"));
    EXPECT_TRUE(git_diff_test::contains(renamed, "rename to new name.txt"));

    const auto deleted = repo.diff("gone.txt");
    EXPECT_TRUE(git_diff_test::contains(deleted, "-bye\n"));
}

TEST_CASE("a repository without commits shows whole files") {
    git_diff_test::Repository repo;
    repo.write("a.txt", "new\n");
    repo.git({"add", "a.txt"});
    EXPECT_TRUE(git_diff_test::contains(repo.diff("a.txt", false), "+new\n"));
}

TEST_CASE("diff lines get colors by kind") {
    const auto text = formatGitDiff("diff --git a/x b/x\n"
                                    "--- a/x\n"
                                    "+++ b/x\n"
                                    "@@ -1 +1 @@\n"
                                    "-old\n"
                                    "+new\n"
                                    " same");
    const auto lines = text.split('\n');
    ASSERT_EQ(lines.size(), 7u);
    ASSERT_EQ(lines.at(0).front().f, Palette::comment);
    ASSERT_EQ(lines.at(1).front().f, Palette::comment);
    ASSERT_EQ(lines.at(2).front().f, Palette::comment);
    ASSERT_EQ(lines.at(3).front().f, Palette::statement);
    ASSERT_EQ(lines.at(4).front().f, Palette::error);
    ASSERT_EQ(lines.at(5).front().f, Palette::string);
    ASSERT_EQ(lines.at(6).front().f, Palette::standard);
}

TEST_SUIT_END
