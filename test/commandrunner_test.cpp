#include "core/icommandrunner.h"
#include "mls-unit-test/unittest.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace command_runner_test {

struct TemporaryDirectory {
    std::filesystem::path path;

    TemporaryDirectory() {
        auto pattern = (std::filesystem::temp_directory_path() /
                        "medit-command-test-XXXXXX")
                           .string();
        const auto *created = mkdtemp(pattern.data());
        if (!created) {
            throw std::runtime_error("Cannot create temporary directory");
        }
        path = std::filesystem::canonical(created);
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

} // namespace command_runner_test

TEST_SUIT_BEGIN(CommandRunner)

TEST_CASE("arguments reach the program unchanged, without a shell") {
    auto runner = createCommandRunner();
    const auto result = runner->run(
        {{"printf", "%s|", "a b", "say \"hi\"", "$HOME", "x\ny", "it's"}});
    ASSERT_EQ(result.exitCode, 0);
    ASSERT_EQ(result.output, "a b|say \"hi\"|$HOME|x\ny|it's|");
    ASSERT_EQ(result.errors, "");
}

TEST_CASE("exit code and stderr are reported separately") {
    auto runner = createCommandRunner();
    const auto result =
        runner->run(Command::shell("echo out; echo err >&2; exit 3"));
    ASSERT_EQ(result.exitCode, 3);
    ASSERT_EQ(result.output, "out\n");
    ASSERT_EQ(result.errors, "err\n");
}

TEST_CASE("working directory is applied") {
    command_runner_test::TemporaryDirectory dir;
    auto runner = createCommandRunner();
    const auto result = runner->run({{"pwd"}, dir.path});
    ASSERT_EQ(result.exitCode, 0);
    ASSERT_EQ(result.output, dir.path.string() + "\n");
}

TEST_CASE("a missing program or empty command fails to start") {
    auto runner = createCommandRunner();
    const auto missing = runner->run({{"medit-no-such-program-xyz"}});
    ASSERT_EQ(missing.exitCode, -1);
    ASSERT_NE(missing.errors, "");

    const auto empty = runner->run({});
    ASSERT_EQ(empty.exitCode, -1);
}

TEST_CASE("streamed output arrives line by line and is not collected") {
    auto runner = createCommandRunner();
    auto lines = std::vector<std::string>{};
    auto errorLines = std::vector<std::string>{};
    const auto result = runner->run(
        Command::shell("printf 'a\\n\\nb\\nlast'; printf 'e1\\ne2\\n' >&2"),
        {
            .output = [&](std::string_view line) { lines.emplace_back(line); },
            .errors =
                [&](std::string_view line) { errorLines.emplace_back(line); },
        });
    ASSERT_EQ(result.exitCode, 0);
    ASSERT_EQ(result.output, "");
    ASSERT_EQ(result.errors, "");
    ASSERT_EQ(lines.size(), 4u);
    ASSERT_EQ(lines.at(0), "a");
    ASSERT_EQ(lines.at(1), "");
    ASSERT_EQ(lines.at(2), "b");
    ASSERT_EQ(lines.at(3), "last"); // No trailing newline.
    ASSERT_EQ(errorLines.size(), 2u);
    ASSERT_EQ(errorLines.at(1), "e2");
}

TEST_CASE("large output on both pipes does not deadlock") {
    auto runner = createCommandRunner();
    // Much larger than a pipe buffer, written to stderr first.
    const auto result = runner->run(Command::shell(
        "head -c 300000 /dev/zero >&2; head -c 300000 /dev/zero"));
    ASSERT_EQ(result.exitCode, 0);
    ASSERT_EQ(result.output.size(), 300000u);
    ASSERT_EQ(result.errors.size(), 300000u);
}

TEST_CASE("standard input is empty and a killed program is reported") {
    auto runner = createCommandRunner();
    const auto cat = runner->run({{"cat"}});
    ASSERT_EQ(cat.exitCode, 0);
    ASSERT_EQ(cat.output, "");

    const auto killed = runner->run(Command::shell("kill -9 $$"));
    ASSERT_EQ(killed.exitCode, 128 + 9);
}

TEST_CASE("git add works for a filename with quotes and shell characters") {
    command_runner_test::TemporaryDirectory dir;
    auto runner = createCommandRunner();
    ASSERT_EQ(runner->run({{"git", "init", "-q"}, dir.path}).exitCode, 0);

    const auto name = std::string{"say \"hi\" $HOME `x`.txt"};
    std::ofstream{dir.path / name} << "content\n";
    const auto add = runner->run({{"git", "add", "--", name}, dir.path});
    ASSERT_EQ(add.exitCode, 0);

    const auto listed =
        runner->run({{"git", "ls-files", "-z"}, dir.path}).output;
    ASSERT_EQ(listed, name + std::string(1, '\0'));
}

TEST_CASE("description joins arguments for display") {
    ASSERT_EQ((Command{{"git", "add", "a b"}}.description()), "git add a b");
}

TEST_SUIT_END
