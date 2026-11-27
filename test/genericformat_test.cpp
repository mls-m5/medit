#include "core/icommandrunner.h"
#include "mls-unit-test/unittest.h"
#include "plugin/genericformat.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace generic_format_test {

/// A directory with fake tools that replaces PATH while it exists.
struct FakeTools {
    std::filesystem::path path;
    std::optional<std::string> previousPath;

    FakeTools() {
        auto pattern = (std::filesystem::temp_directory_path() /
                        "medit-format-test-XXXXXX")
                           .string();
        const auto *created = mkdtemp(pattern.data());
        if (!created) {
            throw std::runtime_error("Cannot create temporary directory");
        }
        path = std::filesystem::canonical(created);
        if (const auto *current = std::getenv("PATH")) {
            previousPath = current;
        }
        setenv("PATH", (path / "bin").c_str(), 1);
        std::filesystem::create_directory(path / "bin");
    }

    ~FakeTools() {
        if (previousPath) {
            setenv("PATH", previousPath->c_str(), 1);
        }
        else {
            unsetenv("PATH");
        }
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }

    void add(const std::string &name, const std::string &script = "") {
        const auto file = path / "bin" / name;
        std::ofstream{file} << "#!/bin/sh\n" << script;
        std::filesystem::permissions(file, std::filesystem::perms::owner_all);
    }
};

std::vector<std::string> arguments(const std::optional<Command> &command) {
    return command ? command->arguments : std::vector<std::string>{};
}

using Arguments = std::vector<std::string>;

} // namespace generic_format_test

TEST_SUIT_BEGIN(GenericFormat)

TEST_CASE("each file type gets its formatter with the path as one argument") {
    using generic_format_test::Arguments;
    generic_format_test::FakeTools tools;
    tools.add("clang-format");
    tools.add("clang-format-17");
    tools.add("tidy");
    tools.add("gofmt");
    tools.add("cmake-format");

    const auto cpp = std::string{"/src/say \"hi\" $x `y`.cpp"};
    EXPECT_TRUE(generic_format_test::arguments(GenericFormat::formatCommand(
                    cpp)) == (Arguments{"clang-format-17", cpp, "-i"}));
    EXPECT_TRUE(generic_format_test::arguments(GenericFormat::formatCommand(
                    "/a b/page.html")) == (Arguments{"tidy",
                                                     "-q",
                                                     "-indent",
                                                     "-m",
                                                     "--tidy-mark",
                                                     "no",
                                                     "/a b/page.html"}));
    EXPECT_TRUE(generic_format_test::arguments(GenericFormat::formatCommand(
                    "/a/data.xml")) == (Arguments{"tidy",
                                                  "-q",
                                                  "-indent",
                                                  "-m",
                                                  "--tidy-mark",
                                                  "no",
                                                  "-xml",
                                                  "/a/data.xml"}));
    EXPECT_TRUE(generic_format_test::arguments(GenericFormat::formatCommand(
                    "/a/main.go")) == (Arguments{"gofmt", "-w", "/a/main.go"}));
    EXPECT_TRUE(generic_format_test::arguments(
                    GenericFormat::formatCommand("/a/CMakeLists.txt")) ==
                (Arguments{"cmake-format", "-i", "/a/CMakeLists.txt"}));
    EXPECT_TRUE(generic_format_test::arguments(GenericFormat::formatCommand(
                    "/a/x.py")) == (Arguments{"black", "-q", "/a/x.py"}));
    EXPECT_TRUE(generic_format_test::arguments(GenericFormat::formatCommand(
                    "/a/x.rs")) == (Arguments{"rustfmt", "/a/x.rs"}));
}

TEST_CASE("missing tools and unknown files give no command") {
    generic_format_test::FakeTools tools; // Empty PATH directory.
    EXPECT_FALSE(GenericFormat::formatCommand("/a/x.cpp").has_value());
    EXPECT_FALSE(GenericFormat::formatCommand("/a/x.html").has_value());
    EXPECT_FALSE(GenericFormat::formatCommand("/a/x.go").has_value());
    EXPECT_FALSE(GenericFormat::formatCommand("/a/CMakeLists.txt").has_value());
    EXPECT_FALSE(GenericFormat::formatCommand("/a/notes.unknown").has_value());

    tools.add("clang-format");
    ASSERT_EQ(
        generic_format_test::arguments(GenericFormat::formatCommand("/a/x.cpp"))
            .front(),
        "clang-format");
}

TEST_CASE("a formatter rewrites the right file even with shell characters") {
    generic_format_test::FakeTools tools;
    // Like gofmt -w: rewrite the file given as the last argument.
    tools.add("gofmt",
              "for last; do :; done\n"
              "printf 'formatted\\n' > \"$last\"\n"
              "echo 'warning from formatter' >&2\n");

    const auto file = tools.path / "say \"hi\" $HOME `x`.go";
    std::ofstream{file} << "package main\n";
    const auto command = GenericFormat::formatCommand(file);
    ASSERT_EQ(command.has_value(), true);

    auto runner = createCommandRunner();
    auto out = std::ostringstream{};
    auto err = std::ostringstream{};
    const auto result = runner->run(*command, CommandOutput::to(out, err));
    ASSERT_EQ(result.exitCode, 0);
    ASSERT_EQ(err.str(), "warning from formatter\n");

    auto content = std::string{};
    std::getline(std::ifstream{file}, content);
    ASSERT_EQ(content, "formatted");
}

TEST_SUIT_END
