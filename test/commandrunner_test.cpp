#include "core/icommandrunner.h"
#include "mls-unit-test/unittest.h"
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
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
        CommandOutput::lines(
            [&](std::string_view line) { lines.emplace_back(line); },
            [&](std::string_view line) { errorLines.emplace_back(line); }));
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

TEST_CASE("raw callbacks receive exactly the bytes written") {
    auto runner = createCommandRunner();
    auto output = std::string{};
    auto finished = 0;
    auto stream = CommandOutput{};
    stream.output = [&](std::string_view chunk) { output.append(chunk); };
    stream.finished = [&] { ++finished; };
    const auto result =
        runner->run(Command::shell("printf 'no newline'"), stream);
    ASSERT_EQ(output, "no newline");
    ASSERT_EQ(result.output, "");
    ASSERT_EQ(finished, 1);
}

TEST_CASE("lines can stream one stream and collect the other") {
    auto runner = createCommandRunner();
    auto lines = std::vector<std::string>{};
    const auto result = runner->run(
        Command::shell("echo a; echo b; echo e >&2"),
        CommandOutput::lines(
            [&](std::string_view line) { lines.emplace_back(line); }, {}));
    ASSERT_EQ(lines.size(), 2u);
    ASSERT_EQ(result.errors, "e\n");
}

TEST_CASE("to() writes both streams to ostreams") {
    auto runner = createCommandRunner();
    auto out = std::ostringstream{};
    auto err = std::ostringstream{};
    const auto result =
        runner->run(Command::shell("printf out; printf err >&2; exit 2"),
                    CommandOutput::to(out, err));
    ASSERT_EQ(result.exitCode, 2);
    ASSERT_EQ(out.str(), "out");
    ASSERT_EQ(err.str(), "err");
}

TEST_CASE("input is written to stdin, also when larger than a pipe") {
    auto runner = createCommandRunner();
    const auto small = runner->run({{"cat"}, {}, "a\nb"});
    ASSERT_EQ(small.output, "a\nb");

    // cat writes while we still write: both directions must progress.
    const auto large = std::string(1000000, 'x');
    const auto echoed = runner->run({{"cat"}, {}, large});
    ASSERT_EQ(echoed.exitCode, 0);
    ASSERT_EQ(echoed.output.size(), large.size());
}

TEST_CASE("a program that ignores its input does not raise SIGPIPE") {
    auto runner = createCommandRunner();
    // The test process does not ignore SIGPIPE: this would kill it.
    const auto result = runner->run({{"true"}, {}, std::string(1000000, 'x')});
    ASSERT_EQ(result.exitCode, 0);
}

TEST_CASE("the child gets default SIGPIPE even if the editor ignores it") {
    auto previous = std::signal(SIGPIPE, SIG_IGN);
    auto runner = createCommandRunner();
    const auto result = runner->run(Command::shell("yes | head -n 1"));
    std::signal(SIGPIPE, previous);
    ASSERT_EQ(result.exitCode, 0);
    ASSERT_EQ(result.output, "y\n");
    ASSERT_EQ(result.errors, ""); // No "Broken pipe" from yes.
}

TEST_CASE("start runs on its own thread and reports through done") {
    auto runner = createCommandRunner();
    auto doneExit = -2;
    auto doneOutput = std::string{};
    auto command = runner->start(Command::shell("echo started; exit 4"),
                                 {},
                                 [&](const CommandResult &result) {
                                     doneExit = result.exitCode;
                                     doneOutput = result.output;
                                 });
    const auto &result = command->wait();
    ASSERT_EQ(result.exitCode, 4);
    ASSERT_EQ(result.output, "started\n");
    ASSERT_EQ(doneExit, 4);
    ASSERT_EQ(doneOutput, "started\n");
}

TEST_CASE("kill stops a shell and the programs it started") {
    using namespace std::chrono;
    auto runner = createCommandRunner();
    const auto begin = steady_clock::now();
    // The shell forks sleep, which keeps the output pipe open.
    auto command = runner->start(Command::shell("sleep 30; echo late"));
    std::this_thread::sleep_for(100ms);
    command->kill();
    const auto &result = command->wait();
    EXPECT_TRUE(steady_clock::now() - begin < 10s);
    ASSERT_EQ(result.exitCode, 128 + SIGTERM);
    ASSERT_EQ(result.output, "");
}

TEST_CASE("destroying a running command stops it") {
    using namespace std::chrono;
    auto runner = createCommandRunner();
    const auto begin = steady_clock::now();
    {
        auto command = runner->start(Command::shell("sleep 30"));
        std::this_thread::sleep_for(50ms);
    }
    EXPECT_TRUE(steady_clock::now() - begin < 10s);

    // Killing before the program has started must not be lost either.
    auto early = runner->start({{"sleep", "30"}});
    early->kill();
    ASSERT_EQ(early->wait().exitCode, 128 + SIGTERM);
}

TEST_CASE("done may destroy its own running command") {
    auto runner = createCommandRunner();
    auto holder = std::unique_ptr<RunningCommand>{};
    auto assigned = std::promise<void>{};
    auto finished = std::promise<int>{};
    auto future = finished.get_future();
    holder = runner->start(
        {{"true"}},
        {},
        [&, isAssigned = assigned.get_future().share()](
            const CommandResult &result) {
            isAssigned.wait(); // Make sure `holder` owns this command.
            holder.reset();    // Destroyed on its own thread.
            finished.set_value(result.exitCode);
        });
    assigned.set_value();
    EXPECT_TRUE(future.wait_for(std::chrono::seconds{10}) ==
                std::future_status::ready);
    ASSERT_EQ(future.get(), 0);
}

TEST_CASE("description joins arguments for display") {
    ASSERT_EQ((Command{{"git", "add", "a b"}}.description()), "git add a b");
}

TEST_SUIT_END
