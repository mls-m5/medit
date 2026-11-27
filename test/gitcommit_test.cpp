#include "core/icommandrunner.h"
#include "files/gitstatus.h"
#include "mls-unit-test/unittest.h"
#include "plugin/gitcommitinteraction.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace git_commit_test {

using namespace std::string_literals;

struct TemporaryRepository {
    std::filesystem::path path;
    std::unique_ptr<ICommandRunner> runner = createCommandRunner();

    TemporaryRepository() {
        auto pattern =
            (std::filesystem::temp_directory_path() / "medit-git-test-XXXXXX")
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

    ~TemporaryRepository() {
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

    std::vector<std::string> filesInHead() {
        auto files = std::vector<std::string>{};
        const auto output = git({"ls-tree", "-r", "--name-only", "-z", "HEAD"});
        for (size_t start = 0, end = 0;
             (end = output.find('\0', start)) != std::string::npos;
             start = end + 1) {
            files.push_back(output.substr(start, end - start));
        }
        std::sort(files.begin(), files.end());
        return files;
    }
};

std::vector<GitStatusEntry> without(std::vector<GitStatusEntry> files,
                                    std::vector<std::string> paths) {
    std::erase_if(files, [&](const auto &file) {
        return std::find(paths.begin(), paths.end(), file.path) != paths.end();
    });
    return files;
}

} // namespace git_commit_test

TEST_SUIT_BEGIN(GitCommit)

TEST_CASE("parse porcelain status with renames and unusual names") {
    using namespace std::string_literals;
    const auto output = "AM mod.txt\0R  new \"x\".txt\0old.txt\0"
                        "?? un tracked.txt\0 D a\nb\0"s;
    const auto entries = parseGitStatus(output);
    ASSERT_EQ(entries.size(), 4u);
    ASSERT_EQ(entries[0].index, 'A');
    ASSERT_EQ(entries[0].worktree, 'M');
    ASSERT_EQ(entries[0].path, "mod.txt");
    ASSERT_EQ(entries[1].path, "new \"x\".txt");
    ASSERT_EQ(entries[1].originalPath, "old.txt");
    ASSERT_EQ(entries[2].path, "un tracked.txt");
    ASSERT_EQ(entries[2].originalPath, "");
    ASSERT_EQ(entries[3].path, "a\nb");
    EXPECT_TRUE(entries[3].isDeleted());
    ASSERT_EQ(parseGitStatus("").size(), 0u);
}

TEST_CASE("files moved above the separator are selected") {
    const auto files = parseGitStatus(
        std::string{"AM mod.txt\0R  new.txt\0old.txt\0?? a b.txt\0", 40});
    ASSERT_EQ(files.size(), 3u);

    const auto text = gitCommitTemplate({}, false, files);
    // By default every file is below the second separator: none selected.
    const auto untouched = parseGitCommitResponse(text, files);
    ASSERT_EQ(untouched.files.size(), 0u);
    ASSERT_EQ(untouched.unknownLines.size(), 0u);

    const auto edited = "Fix it\n\n# a comment\n==============\n"
                        "AM mod.txt\n"
                        "?? a b.txt\n"
                        "?? a b.txt\n" // Twice: still selected once.
                        "============== move files up\n"
                        "R  new.txt <- old.txt\n";
    const auto selection = parseGitCommitResponse(edited, files);
    ASSERT_EQ(selection.message, "Fix it");
    ASSERT_EQ(selection.files.size(), 2u);
    ASSERT_EQ(selection.files[0].path, "mod.txt");
    ASSERT_EQ(selection.files[1].path, "a b.txt");
}

TEST_CASE("removing the second separator selects all; edits are reported") {
    const auto files = parseGitStatus(std::string{" M x.txt\0?? y.txt\0", 18});
    ASSERT_EQ(files.size(), 2u);

    auto text = gitCommitTemplate({}, false, files);
    const auto separator = text.find("============== move");
    text.erase(separator, text.find('\n', separator) - separator + 1);
    text.insert(0, "message");
    const auto all = parseGitCommitResponse(text, files);
    ASSERT_EQ(all.files.size(), 2u);
    ASSERT_EQ(all.message, "message");

    const auto edited =
        parseGitCommitResponse("m\n===\n M x.txt\n M renamed.txt\n", files);
    ASSERT_EQ(edited.files.size(), 1u);
    ASSERT_EQ(edited.unknownLines.size(), 1u);
    ASSERT_EQ(edited.unknownLines[0], " M renamed.txt");
}

TEST_CASE("amend template starts with the previous message") {
    const auto text = gitCommitTemplate("Old message\n", true, {});
    EXPECT_TRUE(text.starts_with("Old message\n"));
    EXPECT_TRUE(text.find("# Amend Commit") != std::string::npos);
}

TEST_CASE("commit stages exactly the selected files, whatever their names") {
    git_commit_test::TemporaryRepository repo;
    repo.write("keep.txt", "1\n");
    repo.write("old name.txt", "rename me\n");
    repo.write("gone.txt", "delete me\n");
    repo.git({"add", "--all"});
    repo.git({"commit", "-q", "-m", "base"});

    repo.write("keep.txt", "2\n");
    repo.git({"mv", "old name.txt", "new \"q\" $x.txt"});
    std::filesystem::remove(repo.path / "gone.txt");
    repo.write("say \"hi\" `x`.txt", "new\n");
    repo.write("not selected.txt", "stay\n");
    repo.write("staged other.txt", "staged\n");
    repo.git({"add", "--", "staged other.txt"});

    const auto files =
        parseGitStatus(repo.git({"status", "--porcelain=v1", "-z"}));
    auto selection = GitCommitSelection{
        .message = "Message with \"quotes\" and $HOME\n\nBody",
        .files = git_commit_test::without(
            files, {"not selected.txt", "staged other.txt"}),
        .unknownLines = {},
    };
    ASSERT_EQ(selection.files.size(), 4u);

    const auto result =
        commitGitSelection(*repo.runner, repo.path, selection, false);
    ASSERT_EQ(result.exitCode, 0);

    const auto inHead = repo.filesInHead();
    ASSERT_EQ(inHead.size(), 3u);
    ASSERT_EQ(inHead[0], "keep.txt");
    ASSERT_EQ(inHead[1], "new \"q\" $x.txt");
    ASSERT_EQ(inHead[2], "say \"hi\" `x`.txt");
    ASSERT_EQ(repo.git({"show", "HEAD:keep.txt"}), "2\n");
    ASSERT_EQ(repo.git({"log", "-1", "--pretty=%B"}),
              "Message with \"quotes\" and $HOME\n\nBody\n\n"); // %B ends with
                                                                // a blank line

    // Unselected files are left as they are, but no longer staged.
    ASSERT_EQ(repo.git({"diff", "--cached", "--name-only"}), "");
    EXPECT_TRUE(std::filesystem::exists(repo.path / "not selected.txt"));
    EXPECT_TRUE(std::filesystem::exists(repo.path / "staged other.txt"));
}

TEST_CASE("amend replaces the message; an empty selection changes nothing") {
    git_commit_test::TemporaryRepository repo;
    repo.write("a.txt", "a\n");
    repo.git({"add", "a.txt"});
    repo.git({"commit", "-q", "-m", "first"});

    repo.write("b.txt", "b\n");
    repo.git({"add", "b.txt"});
    const auto empty =
        commitGitSelection(*repo.runner,
                           repo.path,
                           {.message = "x", .files = {}, .unknownLines = {}},
                           false);
    ASSERT_NE(empty.exitCode, 0);
    ASSERT_EQ(repo.git({"diff", "--cached", "--name-only"}), "b.txt\n");

    const auto files =
        parseGitStatus(repo.git({"status", "--porcelain=v1", "-z"}));
    const auto amended = commitGitSelection(
        *repo.runner,
        repo.path,
        {.message = "second", .files = files, .unknownLines = {}},
        true);
    ASSERT_EQ(amended.exitCode, 0);
    ASSERT_EQ(repo.git({"rev-list", "--count", "HEAD"}), "1\n");
    ASSERT_EQ(repo.git({"log", "-1", "--pretty=%B"}), "second\n\n");
    ASSERT_EQ(repo.filesInHead().size(), 2u);
}

TEST_SUIT_END
