#include "core/icommandrunner.h"
#include "mls-unit-test/unittest.h"
#include "views/console.h"
#include <chrono>
#include <csignal>
#include <thread>

TEST_SUIT_BEGIN(ConsoleCommands)

TEST_CASE("stopAll stops running commands and forget releases them") {
    using namespace std::chrono;
    auto runner = createCommandRunner();
    auto commands = ConsoleCommands{};
    auto exitCode = -2;
    auto *first = runner
                      ->start(Command::shell("sleep 30"),
                              {},
                              [&](auto &r) { exitCode = r.exitCode; })
                      .release();
    const auto id = commands.keep(std::unique_ptr<RunningCommand>{first});
    commands.keep(runner->start({{"sleep", "30"}}));
    ASSERT_EQ(commands.size(), 2u);

    const auto begin = steady_clock::now();
    std::this_thread::sleep_for(50ms);
    commands.stopAll();
    first->wait();
    EXPECT_TRUE(steady_clock::now() - begin < 10s);
    ASSERT_EQ(exitCode, 128 + SIGTERM);

    commands.forget(id);
    ASSERT_EQ(commands.size(), 1u);
    commands.forget(12345); // Unknown ids are ignored.
    ASSERT_EQ(commands.size(), 1u);
}

TEST_CASE("destroying the console commands stops what still runs") {
    using namespace std::chrono;
    auto runner = createCommandRunner();
    const auto begin = steady_clock::now();
    {
        auto commands = ConsoleCommands{};
        commands.keep(runner->start(Command::shell("sleep 30")));
        std::this_thread::sleep_for(50ms);
    }
    EXPECT_TRUE(steady_clock::now() - begin < 10s);
}

TEST_SUIT_END
