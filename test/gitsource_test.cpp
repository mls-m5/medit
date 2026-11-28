#include "core/icommandrunner.h"
#include "files/filesource.h"
#include "files/ifilesystem.h"
#include "mls-unit-test/unittest.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace git_source_test {

struct Directory {
    std::filesystem::path path;

    Directory() {
        auto pattern =
            (std::filesystem::temp_directory_path() / "medit-git-source-XXXXXX")
                .string();
        if (!mkdtemp(pattern.data())) {
            throw std::runtime_error("Cannot create temporary directory");
        }
        path = std::filesystem::canonical(pattern);
    }

    ~Directory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

struct Git {
    std::unique_ptr<ICommandRunner> runner = createCommandRunner();

    void operator()(const std::filesystem::path &directory,
                    std::vector<std::string> arguments) {
        arguments.insert(arguments.begin(), "git");
        const auto result = runner->run({arguments, directory});
        if (result.exitCode) {
            throw std::runtime_error("git failed: " + result.errors);
        }
    }

    void init(const std::filesystem::path &directory) {
        std::filesystem::create_directories(directory);
        (*this)(directory, {"init", "-q"});
        (*this)(directory, {"config", "user.name", "Test"});
        (*this)(directory, {"config", "user.email", "test@example.com"});
        (*this)(directory, {"config", "commit.gpgsign", "false"});
    }
};

void write(const std::filesystem::path &path, const std::string &content = "") {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream{path} << content;
}

/// The listed files relative to `root`, sorted.
std::vector<std::string> relative(const ProjectFiles &files,
                                  const std::filesystem::path &root) {
    auto ret = std::vector<std::string>{};
    for (auto &file : files.files) {
        ret.push_back(file.lexically_relative(root).generic_string());
    }
    std::sort(ret.begin(), ret.end());
    return ret;
}

} // namespace git_source_test

TEST_SUIT_BEGIN(GitSource)

TEST_CASE("lists tracked and untracked files, not ignored or deleted ones") {
    git_source_test::Directory dir;
    git_source_test::Git git;
    const auto root = dir.path;
    git.init(root);
    git_source_test::write(root / "tracked.cpp");
    git_source_test::write(root / "gone.cpp");
    git_source_test::write(root / ".gitignore", "ignored.txt\nbuild/\n");
    git(root, {"add", "--all"});
    git(root, {"commit", "-q", "-m", "base"});
    std::filesystem::remove(root / "gone.cpp");
    git_source_test::write(root / "ignored.txt");
    git_source_test::write(root / "build" / "out.o");
    git_source_test::write(root / "src" / "new file.txt");
    git_source_test::write(root / "say \"hi\"\nthere.md");
    // Any extension: Git decides what belongs to the project.
    git_source_test::write(root / "LICENSE");

    auto source = createGitSource(*git.runner, localFileSystem());
    const auto files = source->list(root, 1000);
    EXPECT_FALSE(files.fellBack);
    EXPECT_FALSE(files.truncated);
    EXPECT_TRUE((git_source_test::relative(files, root) ==
                 std::vector<std::string>{".gitignore",
                                          "LICENSE",
                                          "say \"hi\"\nthere.md",
                                          "src/new file.txt",
                                          "tracked.cpp"}));
}

TEST_CASE("submodules and nested repositories are listed with their files") {
    git_source_test::Directory dir;
    git_source_test::Git git;
    const auto upstream = dir.path / "upstream";
    git.init(upstream);
    git_source_test::write(upstream / "lib.cpp");
    git(upstream, {"add", "--all"});
    git(upstream, {"commit", "-q", "-m", "lib"});

    const auto root = dir.path / "project";
    git.init(root);
    git_source_test::write(root / "main.cpp");
    git(root,
        {"-c",
         "protocol.file.allow=always",
         "submodule",
         "add",
         "-q",
         upstream.string(),
         "lib"});
    git.init(root / "nested");
    git_source_test::write(root / "nested" / "inner.cpp");

    auto source = createGitSource(*git.runner, localFileSystem());
    const auto files =
        git_source_test::relative(source->list(root, 1000), root);
    EXPECT_TRUE((files == std::vector<std::string>{".gitmodules",
                                                   "lib/lib.cpp",
                                                   "main.cpp",
                                                   "nested/inner.cpp"}));
}

TEST_CASE("outside a repository the directory is listed instead") {
    git_source_test::Directory dir;
    git_source_test::Git git;
    git_source_test::write(dir.path / "a.cpp");
    git_source_test::write(dir.path / "notes.unknownextension");
    auto source = createGitSource(*git.runner, localFileSystem());
    const auto files = source->list(dir.path, 1000);
    EXPECT_TRUE(files.fellBack);
    EXPECT_TRUE((git_source_test::relative(files, dir.path) ==
                 std::vector<std::string>{"a.cpp"}));
}

TEST_CASE("the limit marks the list as truncated") {
    git_source_test::Directory dir;
    git_source_test::Git git;
    git.init(dir.path);
    for (auto name : {"a", "b", "c"}) {
        git_source_test::write(dir.path / name);
    }
    auto source = createGitSource(*git.runner, localFileSystem());
    const auto files = source->list(dir.path, 2);
    ASSERT_EQ(files.files.size(), 2u);
    EXPECT_TRUE(files.truncated);
}

TEST_SUIT_END
