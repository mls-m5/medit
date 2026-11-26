#include "core/jobqueue.h"
#include "files/project.h"
#include "helpers/temporaryproject.h"
#include "linux/inotify.h"
#include "mls-unit-test/unittest.h"
#include "views/sidebar.h"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <sstream>

#ifdef MEDIT_USING_LINUX

namespace inotify_test {
using namespace std::chrono_literals;
using Path = std::filesystem::path;

// Execute callbacks on the test/GUI thread. Wait for submitted work, not fixed
// sleeps, and fail with the operation description after a bounded deadline.
class GuiQueue : public JobQueue {
    std::mutex _mutex;
    std::condition_variable _ready;
    size_t _submitted = 0;

public:
    void addTask(std::function<void()> task) override {
        JobQueue::addTask(std::move(task));
        {
            auto lock = std::lock_guard{_mutex};
            ++_submitted;
        }
        _ready.notify_all();
    }

    template <class Predicate>
    void until(Predicate predicate, std::string_view description) {
        const auto deadline = std::chrono::steady_clock::now() + 4s;
        while (true) {
            size_t before;
            {
                auto lock = std::lock_guard{_mutex};
                before = _submitted;
            }
            work(false);
            if (predicate())
                return;
            if (std::chrono::steady_clock::now() >= deadline)
                throw std::runtime_error("Timed out waiting for inotify: " +
                                         std::string{description});
            auto lock = std::unique_lock{_mutex};
            if (!_ready.wait_until(
                    lock, deadline, [&] { return _submitted != before; }))
                throw std::runtime_error("Timed out waiting for inotify: " +
                                         std::string{description});
        }
    }

    size_t submitted() {
        auto lock = std::lock_guard{_mutex};
        return _submitted;
    }

    void pending(size_t after = 0) {
        auto lock = std::unique_lock{_mutex};
        if (!_ready.wait_for(lock, 4s, [&] { return _submitted > after; }))
            throw std::runtime_error(
                "No queued inotify callback before deadline");
    }
};

struct Fixture {
    test_helpers::TemporaryProject environment;
    GuiQueue gui;
    std::unique_ptr<DirectoryNotifications> notifications =
        createDirectoryNotifications(gui);
    std::unique_ptr<Project> project;

    void start() {
        project = std::make_unique<Project>(
            environment.path(".medit.json"), *notifications, gui);
        project->updateCache();
    }
    bool contains(const Path &relative) const {
        const auto &files = project->files();
        return std::find(files.begin(),
                         files.end(),
                         environment.path(relative)) != files.end();
    }
    void wait(const Path &relative, bool present = true) {
        gui.until([&] { return contains(relative) == present; },
                  relative.string() + (present ? " added" : " removed"));
    }
};
} // namespace inotify_test

TEST_SUIT_BEGIN(Inotify)

TEST_CASE("existing nested directories publish add rename and remove to the "
          "project and sidebar") {
    inotify_test::Fixture f;
    f.environment.write("src/deep/existing.cpp");
    f.start();
    SideBar sidebar{nullptr};
    sidebar.project(*f.project);
    EXPECT_TRUE(
        sidebar.tree().reveal(f.environment.path("src/deep/existing.cpp")));
    f.environment.write("src/deep/new.cpp");
    f.wait("src/deep/new.cpp");
    EXPECT_TRUE(sidebar.tree().reveal(f.environment.path("src/deep/new.cpp")));
    std::filesystem::rename(f.environment.path("src/deep/new.cpp"),
                            f.environment.path("src/deep/renamed.cpp"));
    f.wait("src/deep/renamed.cpp");
    f.wait("src/deep/new.cpp", false);
    std::filesystem::remove(f.environment.path("src/deep/renamed.cpp"));
    f.wait("src/deep/renamed.cpp", false);
    EXPECT_FALSE(
        sidebar.tree().reveal(f.environment.path("src/deep/renamed.cpp")));
}

TEST_CASE(
    "new directories populated immediately are reconciled and then watched") {
    inotify_test::Fixture f;
    f.start();
    f.environment.write("new/deep/first.cpp");
    f.wait("new/deep/first.cpp");
    f.environment.write("new/deep/second.cpp");
    f.wait("new/deep/second.cpp");
    std::filesystem::remove_all(f.environment.path("new"));
    f.wait("new/deep/first.cpp", false);
    f.wait("new/deep/second.cpp", false);
}

TEST_CASE(
    "moving populated directories in and out reconciles all descendants") {
    inotify_test::Fixture f;
    f.environment.write("../incoming/deep/one.cpp");
    f.start();
    std::filesystem::rename(f.environment.path("../incoming"),
                            f.environment.path("moved"));
    f.wait("moved/deep/one.cpp");
    f.environment.write("moved/deep/two.cpp");
    f.wait("moved/deep/two.cpp");
    std::filesystem::rename(f.environment.path("moved"),
                            f.environment.path("../outgoing"));
    f.wait("moved/deep/one.cpp", false);
    f.wait("moved/deep/two.cpp", false);
    bool staleEvent = false;
    f.notifications->subscribe(
        [&](auto, auto path, auto) {
            if (path == f.environment.path("moved/deep/outside.cpp"))
                staleEvent = true;
        },
        &staleEvent);
    f.environment.write("../outgoing/deep/outside.cpp");
    f.environment.write("probe.cpp");
    f.wait("probe.cpp");
    EXPECT_FALSE(staleEvent);
}

TEST_CASE(
    "renaming a directory inside the root updates descendant watch paths") {
    inotify_test::Fixture f;
    f.environment.write("old/deep/one.cpp");
    f.start();
    std::filesystem::rename(f.environment.path("old"),
                            f.environment.path("renamed"));
    f.wait("renamed/deep/one.cpp");
    f.wait("old/deep/one.cpp", false);
    f.environment.write("renamed/deep/two.cpp");
    f.wait("renamed/deep/two.cpp");
}

TEST_CASE(
    "content-only callbacks run on the GUI thread without membership changes") {
    inotify_test::Fixture f;
    f.environment.write("src/existing.cpp");
    f.start();
    size_t membership = 0;
    bool changed = false, correctThread = false;
    const auto guiThread = std::this_thread::get_id();
    f.project->subscribeToFilesChanged([&] { ++membership; }, &membership);
    f.notifications->subscribe(
        [&](auto type, auto path, auto) {
            if (type == DirectoryNotifications::Changed &&
                path == f.environment.path("src/existing.cpp")) {
                changed = true;
                correctThread = std::this_thread::get_id() == guiThread;
            }
        },
        &changed);
    f.environment.write("src/existing.cpp", "// changed\n");
    f.gui.until([&] { return changed; },
                "content change delivered on GUI thread");
    ASSERT_EQ(membership, 0u);
    EXPECT_TRUE(correctThread);
}

TEST_CASE("changing roots cancels queued old events and watches the new root") {
    inotify_test::Fixture f;
    const auto other = f.environment.path("../other");
    std::filesystem::create_directory(other);
    std::vector<inotify_test::Path> received;
    f.notifications->subscribe(
        [&](auto, auto path, auto) { received.push_back(path); }, &received);
    f.notifications->path(f.environment.root);
    f.environment.write("old.cpp");
    f.gui.pending();
    f.notifications->path(other);
    f.gui.work(false);
    EXPECT_TRUE(received.empty());
    f.environment.write("../other/new.cpp");
    f.gui.until(
        [&] {
            return std::find(received.begin(),
                             received.end(),
                             other / "new.cpp") != received.end();
        },
        "new root event");
    f.environment.write("obsolete.cpp");
    f.environment.write("../other/probe.cpp");
    f.gui.until(
        [&] {
            return std::find(received.begin(),
                             received.end(),
                             other / "probe.cpp") != received.end();
        },
        "new root probe");
    for (const auto &path : received)
        EXPECT_TRUE(path.parent_path() == other);
}

TEST_CASE(
    "configuration changes move the project's live watch to the new root") {
    inotify_test::Fixture f;
    f.environment.write("old.cpp");
    f.environment.write("../other/initial.cpp");
    f.start();
    const auto other = f.environment.path("../other");
    f.environment.write(".medit.json", "{\"root\":\"" + other.string() + "\"}");
    f.wait("../other/initial.cpp");
    EXPECT_FALSE(f.contains("old.cpp"));
    f.environment.write("../other/deep/live.cpp");
    f.wait("../other/deep/live.cpp");
    f.environment.write("obsolete.cpp");
    f.environment.write("../other/probe.cpp");
    f.wait("../other/probe.cpp");
    EXPECT_FALSE(f.contains("obsolete.cpp"));
}

TEST_CASE(
    "destruction and empty-root stop invalidate queued callbacks promptly") {
    inotify_test::Fixture f;
    size_t calls = 0;
    f.notifications->subscribe([&](auto, auto, auto) { ++calls; }, &calls);
    f.notifications->path(f.environment.root);
    f.environment.write("queued.cpp");
    f.gui.pending();
    const auto started = std::chrono::steady_clock::now();
    f.notifications->path({});
    f.gui.work(false);
    ASSERT_EQ(calls, 0u);
    const auto before = f.gui.submitted();
    f.notifications->path(f.environment.root);
    f.environment.write("queued2.cpp");
    f.gui.pending(before);
    f.notifications.reset();
    f.gui.work(false);
    ASSERT_EQ(calls, 0u);
    EXPECT_TRUE(std::chrono::steady_clock::now() - started <
                std::chrono::seconds{2});
}

TEST_CASE("subscriptions may unsubscribe during dispatch") {
    inotify_test::Fixture f;
    size_t first = 0, removed = 0;
    f.notifications->subscribe(
        [&](auto, auto, auto) {
            ++first;
            f.notifications->unsubcribe(&first);
            f.notifications->unsubcribe(&removed);
        },
        &first);
    f.notifications->subscribe([&](auto, auto, auto) { ++removed; }, &removed);
    f.notifications->path(f.environment.root);
    f.environment.write("event.cpp");
    f.gui.until([&] { return first > 0; }, "subscription callback");
    ASSERT_EQ(first, 1u);
    ASSERT_EQ(removed, 0u);
}

TEST_CASE("an unavailable root is retried and its files are recovered") {
    inotify_test::Fixture f;
    const auto later = f.environment.path("../later");
    f.environment.write(".medit.json", "{\"root\":\"" + later.string() + "\"}");
    f.start();
    f.environment.write("../later/deep/recovered.cpp");
    f.wait("../later/deep/recovered.cpp");
    f.environment.write("../later/deep/next.cpp");
    f.wait("../later/deep/next.cpp");
}

TEST_CASE("a moved-away root can be recreated at its original path") {
    inotify_test::Fixture f;
    f.environment.write("old.cpp");
    f.start();
    std::filesystem::rename(f.environment.root,
                            f.environment.directory / "gone");
    f.gui.until([&] { return f.project->files().empty(); },
                "moved root removed from index");
    f.environment.write(".medit.json", R"({"build":"matmake"})");
    f.environment.write("new/deep/reborn.cpp");
    f.wait("new/deep/reborn.cpp");
    EXPECT_FALSE(f.contains("old.cpp"));
}

TEST_CASE("directory symlinks and cycles are not traversed") {
    inotify_test::Fixture f;
    f.environment.write("../outside/old.cpp");
    std::filesystem::create_directory_symlink(f.environment.path("../outside"),
                                              f.environment.path("link"));
    std::filesystem::create_directory_symlink(f.environment.root,
                                              f.environment.path("cycle"));
    f.start();
    bool followed = false;
    f.notifications->subscribe(
        [&](auto, auto path, auto) {
            if (path == f.environment.path("link/new.cpp"))
                followed = true;
        },
        &followed);
    f.environment.write("../outside/new.cpp");
    f.environment.write("probe.cpp");
    f.wait("probe.cpp");
    EXPECT_FALSE(f.contains("link/old.cpp"));
    EXPECT_FALSE(f.contains("link/new.cpp"));
    EXPECT_FALSE(followed);
}

TEST_SUIT_END

#endif
