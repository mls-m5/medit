#include "mls-unit-test/unittest.h"
#include "settings.h"
#include <initializer_list>
#include <optional>

namespace settings_test {

Settings parse(std::initializer_list<std::string> options = {}) {
    std::vector<std::string> arguments{"medit"};
    arguments.insert(arguments.end(), options.begin(), options.end());
    std::vector<char *> argv;
    for (auto &argument : arguments)
        argv.push_back(argument.data());
    return Settings{static_cast<int>(argv.size()), argv.data()};
}

#if defined(__linux__) && !defined(__EMSCRIPTEN__)
struct EnvironmentVariable {
    const char *name;
    std::optional<std::string> previous;

    explicit EnvironmentVariable(const char *name)
        : name{name} {
        if (const auto *value = std::getenv(name))
            previous = value;
        set(nullptr);
    }
    ~EnvironmentVariable() {
        set(previous ? previous->c_str() : nullptr);
    }
    void set(const char *value) {
        if (value)
            setenv(name, value, 1);
        else
            unsetenv(name);
    }
};

struct DisplayEnvironment {
    EnvironmentVariable display{"DISPLAY"};
    EnvironmentVariable wayland{"WAYLAND_DISPLAY"};
};
#endif

} // namespace settings_test

TEST_SUIT_BEGIN(Settings)

#if defined(__linux__) && !defined(__EMSCRIPTEN__)

TEST_CASE("no graphical display defaults to terminal mode") {
    settings_test::DisplayEnvironment environment;
    EXPECT_TRUE(settings_test::parse().style == UiStyle::Terminal);
}

TEST_CASE("an X11 display defaults to the GUI even over SSH") {
    settings_test::DisplayEnvironment environment;
    settings_test::EnvironmentVariable ssh{"SSH_CONNECTION"};
    ssh.set("192.0.2.1 12345 192.0.2.2 22");
    environment.display.set("localhost:10.0");
    EXPECT_TRUE(settings_test::parse().style == UiStyle::Matgui);
}

TEST_CASE("a Wayland display defaults to the GUI without X11") {
    settings_test::DisplayEnvironment environment;
    environment.wayland.set("wayland-0");
    EXPECT_TRUE(settings_test::parse().style == UiStyle::Matgui);
}

TEST_CASE("empty display variables do not count as a graphical session") {
    settings_test::DisplayEnvironment environment;
    environment.display.set("");
    environment.wayland.set("");
    EXPECT_TRUE(settings_test::parse().style == UiStyle::Terminal);
    environment.wayland.set("wayland-0");
    EXPECT_TRUE(settings_test::parse().style == UiStyle::Matgui);
    environment.wayland.set("");
    environment.display.set(":0");
    EXPECT_TRUE(settings_test::parse().style == UiStyle::Matgui);
}

TEST_CASE("explicit GUI mode overrides headless detection") {
    settings_test::DisplayEnvironment environment;
    EXPECT_TRUE(settings_test::parse({"--gui"}).style == UiStyle::Matgui);
}

TEST_CASE("explicit CLI mode overrides available displays") {
    settings_test::DisplayEnvironment environment;
    environment.display.set(":0");
    environment.wayland.set("wayland-0");
    EXPECT_TRUE(settings_test::parse({"--cli"}).style == UiStyle::Terminal);
}

TEST_CASE("explicit remote client and server modes remain unchanged") {
    settings_test::DisplayEnvironment environment;
    EXPECT_TRUE(settings_test::parse({"--remote"}).style == UiStyle::Remote);
    EXPECT_TRUE(settings_test::parse({"--connect", "fifo"}).style ==
                UiStyle::FifoClient);
    EXPECT_TRUE(settings_test::parse({"--connect", "tcp"}).style ==
                UiStyle::TcpClient);
    EXPECT_TRUE(settings_test::parse({"--host", "fifo"}).style ==
                UiStyle::FifoServer);
    EXPECT_TRUE(settings_test::parse({"--host", "tcp"}).style ==
                UiStyle::TcpServer);
}

TEST_CASE("the last explicit UI mode still wins") {
    settings_test::DisplayEnvironment environment;
    EXPECT_TRUE(settings_test::parse({"--gui", "--cli"}).style ==
                UiStyle::Terminal);
    EXPECT_TRUE(settings_test::parse({"--cli", "--gui"}).style ==
                UiStyle::Matgui);
}

TEST_CASE("automatic terminal mode preserves filenames and other options") {
    settings_test::DisplayEnvironment environment;
    const auto settings = settings_test::parse(
        {"--profiling", "--port", "1234", "--addr", "example", "notes.md"});
    EXPECT_TRUE(settings.style == UiStyle::Terminal);
    ASSERT_EQ(settings.file, std::filesystem::path{"notes.md"});
    ASSERT_EQ(settings.port, 1234);
    ASSERT_EQ(settings.address, "example");
    EXPECT_TRUE(settings.enablePerformanceProfiling);
}

TEST_CASE("default detection does not require an argument array") {
    settings_test::DisplayEnvironment environment;
    const Settings settings{0, nullptr};
    EXPECT_TRUE(settings.style == UiStyle::Terminal);
}

#else

TEST_CASE("non-Linux builds retain their graphical default") {
    const Settings settings{0, nullptr};
    EXPECT_TRUE(settings.style == UiStyle::Matgui);
}

#endif

TEST_SUIT_END
