#include "core/coreenvironment.h"
#include "core/files.h"
#include "core/jobqueue.h"
#include "core/meditlog.h"
#include "core/timer.h"
#include "mls-unit-test/unittest.h"
#include "text/buffer.h"
#include "text/cursorops.h"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>
#include <stdexcept>
#include <thread>

// Integration tests with the real watcher, a temporary directory and bounded
// waits.
namespace file_watch_test {

std::filesystem::path temporaryDirectory(const char *name) {
    auto pattern = (std::filesystem::temp_directory_path() / name).string();
    if (!mkdtemp(pattern.data())) {
        throw std::runtime_error("Cannot create temporary directory");
    }
    return std::filesystem::canonical(pattern);
}

/// CoreEnvironment keeps FIFOs and history under HOME.
struct TemporaryHome {
    std::optional<std::string> previous;
    std::filesystem::path path = temporaryDirectory("medit-watch-home-XXXXXX");

    TemporaryHome() {
        if (const auto *home = std::getenv("HOME")) {
            previous = home;
        }
        setenv("HOME", path.c_str(), 1);
    }

    ~TemporaryHome() {
        if (previous) {
            setenv("HOME", previous->c_str(), 1);
        }
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

void write(const std::filesystem::path &path, const std::string &content) {
    std::ofstream{path, std::ios::binary} << content;
}

struct Fixture {
    TemporaryHome home;
    std::filesystem::path project =
        temporaryDirectory("medit-watch-project-XXXXXX");
    JobQueue jobs;
    JobQueue gui;
    Timer timer;
    ThreadContext context{jobs, gui, timer};
    CoreEnvironment core{context, project};

    Fixture() {
        std::filesystem::create_directory(project / "sub");
        timer.start();
        unsubscribeToLog();
    }

    ~Fixture() {
        core.files().stopWatching();
        timer.stop();
        std::error_code error;
        std::filesystem::remove_all(project, error);
    }

    /// Runs the queues on this thread until `done` or about three seconds.
    bool runUntil(const std::function<bool()> &done) {
        using namespace std::chrono_literals;
        for (int i = 0; i < 300; ++i) {
            jobs.work(false);
            gui.work(false);
            if (done()) {
                return true;
            }
            std::this_thread::sleep_for(10ms);
        }
        return false;
    }

    /// Runs the queues for a while, for checking that nothing happens.
    void runFor(std::chrono::milliseconds duration) {
        const auto end = std::chrono::steady_clock::now() + duration;
        while (std::chrono::steady_clock::now() < end) {
            jobs.work(false);
            gui.work(false);
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
    }
};

} // namespace file_watch_test

TEST_SUIT_BEGIN(FileWatch)

TEST_CASE("an unchanged buffer follows its file, also in a subdirectory") {
    file_watch_test::Fixture f;
    const auto path = f.project / "sub" / "a.txt";
    file_watch_test::write(path, "1\n2\n");
    auto buffer = f.core.files().open(path);
    ASSERT_EQ(buffer->text(), "1\n2\n");

    file_watch_test::write(path, "1\ntwo\n");
    EXPECT_TRUE(f.runUntil([&] { return buffer->text() == "1\ntwo\n"; }));
    EXPECT_FALSE(buffer->isChanged());

    // Saved by writing a new file and renaming it over the old one.
    file_watch_test::write(f.project / "sub" / "a.txt.tmp", "renamed\n");
    std::filesystem::rename(f.project / "sub" / "a.txt.tmp", path);
    EXPECT_TRUE(f.runUntil([&] { return buffer->text() == "renamed\n"; }));
}

TEST_CASE("unsaved changes are never replaced by a change on disk") {
    file_watch_test::Fixture f;
    const auto path = f.project / "a.txt";
    file_watch_test::write(path, "1\n2\n");
    auto buffer = f.core.files().open(path);
    insert(Cursor{*buffer, 0, 0}, FString{"mine "});

    file_watch_test::write(path, "1\ntheirs\n");
    EXPECT_TRUE(f.runUntil(
        [&] { return buffer->diskState() == DiskState::ChangedOnDisk; }));
    ASSERT_EQ(buffer->text(), "mine 1\n2\n");
}

TEST_CASE("typing right after saving is kept") {
    // The change event of our own save arrives after the save; it must not
    // reload the file over what was typed since.
    file_watch_test::Fixture f;
    const auto path = f.project / "a.txt";
    file_watch_test::write(path, "1\n");
    auto buffer = f.core.files().open(path);
    insert(Cursor{*buffer, 0, 0}, FString{"a"});
    EXPECT_TRUE(buffer->save());
    insert(Cursor{*buffer, 0, 0}, FString{"b"});

    f.runFor(std::chrono::milliseconds{400});
    ASSERT_EQ(buffer->text(), "ba1\n");
    EXPECT_TRUE(buffer->diskState() == DiskState::InSync);
    EXPECT_TRUE(buffer->isChanged());
}

TEST_SUIT_END
