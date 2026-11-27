#include "core/fifofile.h"
#include "core/os.h"
#include "mls-unit-test/unittest.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

namespace os_test {

struct TemporaryDirectory {
    std::filesystem::path path;

    TemporaryDirectory() {
        auto pattern =
            (std::filesystem::temp_directory_path() / "medit-os-test-XXXXXX")
                .string();
        const auto *created = mkdtemp(pattern.data());
        if (!created) {
            throw std::runtime_error("Cannot create temporary directory");
        }
        path = created;
    }

    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

/// Replaces PATH for the lifetime of the object.
struct ScopedPath {
    std::optional<std::string> previous;

    explicit ScopedPath(const std::string &value) {
        if (const auto *path = std::getenv("PATH")) {
            previous = path;
        }
        setenv("PATH", value.c_str(), 1);
    }

    ~ScopedPath() {
        if (previous) {
            setenv("PATH", previous->c_str(), 1);
        }
        else {
            unsetenv("PATH");
        }
    }
};

void writeFile(const std::filesystem::path &path, bool executable) {
    std::ofstream{path} << "#!/bin/sh\n";
    std::filesystem::permissions(path,
                                 executable
                                     ? std::filesystem::perms::owner_all
                                     : std::filesystem::perms::owner_read |
                                           std::filesystem::perms::owner_write);
}

} // namespace os_test

TEST_SUIT_BEGIN(Os)

TEST_CASE("hasCommand finds programs in PATH and rejects missing ones") {
    EXPECT_TRUE(hasCommand("sh"));
    EXPECT_FALSE(hasCommand("medit-no-such-program-xyz"));
    EXPECT_FALSE(hasCommand(""));
}

TEST_CASE("hasCommand only accepts executable regular files") {
    os_test::TemporaryDirectory dir;
    os_test::writeFile(dir.path / "tool", true);
    os_test::writeFile(dir.path / "plain", false);
    std::filesystem::create_directory(dir.path / "folder");

    const auto path = os_test::ScopedPath{"/nonexistent:" + dir.path.string()};
    EXPECT_TRUE(hasCommand("tool"));
    EXPECT_FALSE(hasCommand("plain"));
    EXPECT_FALSE(hasCommand("folder"));

    EXPECT_TRUE(hasCommand((dir.path / "tool").string()));
    EXPECT_FALSE(hasCommand((dir.path / "plain").string()));
}

TEST_CASE("hasCommand does not run the name through a shell") {
    os_test::TemporaryDirectory dir;
    const auto marker = dir.path / "marker";
    // With `command -v` in a shell, this would create the marker file.
    EXPECT_FALSE(hasCommand("x; touch " + marker.string()));
    EXPECT_FALSE(std::filesystem::exists(marker));
}

TEST_CASE("createFifo replaces an existing file and reports failures") {
    os_test::TemporaryDirectory dir;
    const auto path = dir.path / "pipe \"with\" quotes";
    std::ofstream{path} << "old\n";

    ASSERT_EQ(createFifo(path), path);
    EXPECT_TRUE(std::filesystem::is_fifo(path));
    struct stat info = {};
    ASSERT_EQ(::stat(path.c_str(), &info), 0);
    ASSERT_EQ((info.st_mode & 0777u), 0600u);

    auto message = std::string{};
    try {
        createFifo(dir.path / "missing" / "pipe");
    }
    catch (const std::runtime_error &error) {
        message = error.what();
    }
    EXPECT_TRUE(message.find("missing/pipe") != std::string::npos);
}

TEST_SUIT_END
