#include "core/jobqueue.h"
#include "files/project.h"
#include "mls-unit-test/unittest.h"
#include "mock/files/mockdirectorynotifications.h"
#include "views/sidebar.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>

namespace project_test {

struct Environment {
    std::optional<std::string> previousHome;
    std::filesystem::path directory;
    std::filesystem::path root;

    Environment() {
        if (const auto *home = std::getenv("HOME")) {
            previousHome = home;
        }
        auto pattern = (std::filesystem::temp_directory_path() /
                        "medit-project-test-XXXXXX")
                           .string();
        const auto *created = mkdtemp(pattern.data());
        if (!created) {
            throw std::runtime_error("Cannot create temporary project");
        }
        directory = created;
        root = directory / "project";
        std::filesystem::create_directory(root);
        std::ofstream{root / ".medit.json"} << R"({"build":"matmake"})";
        setenv("HOME", directory.c_str(), 1);
    }

    ~Environment() {
        if (previousHome) {
            setenv("HOME", previousHome->c_str(), 1);
        }
        else {
            unsetenv("HOME");
        }
        std::error_code error;
        std::filesystem::remove_all(directory, error);
    }
};

struct Fixture {
    Environment environment;
    MockDirectoryNotifications notifications;
    JobQueue queue;
    DirectoryNotifications::CallbackT notify;
    void *notificationRef = nullptr;
    std::unique_ptr<Project> project;

    explicit Fixture(bool readCache = true) {
        notifications.mock_subscribe_2.expectNum(1);
        notifications.mock_subscribe_2.onCall([this](auto callback, void *ref) {
            notify = std::move(callback);
            notificationRef = ref;
        });
        notifications.mock_unsubcribe_1.expectNum(1);
        notifications.mock_unsubcribe_1.expectArgs(
            [this](void *ref) { return ref == notificationRef; });
        project = std::make_unique<Project>(
            environment.root / ".medit.json", notifications, queue);
        if (readCache) {
            project->updateCache();
        }
    }

    std::filesystem::path path(std::filesystem::path relative) const {
        return (environment.root / relative).lexically_normal();
    }

    void write(std::filesystem::path relative, std::string_view text = "") {
        const auto file = path(relative);
        std::filesystem::create_directories(file.parent_path());
        std::ofstream{file} << text;
    }

    void event(DirectoryNotifications::EventType type,
               std::filesystem::path relative) {
        notify(type, environment.root / relative, {});
    }

    void drain() {
        queue.work(false);
    }

    bool contains(std::filesystem::path relative) const {
        const auto &files = project->files();
        return std::find(files.begin(), files.end(), path(relative)) !=
               files.end();
    }
};

} // namespace project_test

TEST_SUIT_BEGIN(Project)

TEST_CASE("load and refresh publish a complete deduplicated list") {
    project_test::Fixture f{false};
    int calls = 0;
    f.project->subscribeToFilesChanged([&] { ++calls; }, &calls);
    ASSERT_EQ(calls, 0);
    f.project->updateCache();
    ASSERT_EQ(calls, 1);
    ASSERT_EQ(f.project->settings().root, f.environment.root);
    ASSERT_EQ(f.project->settings().buildCommand, "matmake");
    ASSERT_EQ(f.project->files().size(), 1u);
    EXPECT_TRUE(f.contains(".medit.json"));
    f.project->updateCache();
    ASSERT_EQ(calls, 1);
    f.write("new.cpp");
    f.project->updateCache();
    ASSERT_EQ(calls, 2);
    EXPECT_TRUE(f.contains("new.cpp"));
}

TEST_CASE("added events publish updated files on the GUI queue") {
    project_test::Fixture f;
    int calls = 0;
    std::vector<std::filesystem::path> observed;
    f.project->subscribeToFilesChanged(
        [&] {
            ++calls;
            observed = f.project->files();
        },
        &calls);
    f.write("src/new.cpp");
    f.event(DirectoryNotifications::Added, "src/new.cpp");
    ASSERT_EQ(calls, 0);
    EXPECT_FALSE(f.contains("src/new.cpp"));
    f.drain();
    ASSERT_EQ(calls, 1);
    EXPECT_TRUE(f.contains("src/new.cpp"));
    EXPECT_TRUE(observed == f.project->files());
}

TEST_CASE("duplicate and equivalent added paths do not notify twice") {
    project_test::Fixture f;
    int calls = 0;
    f.project->subscribeToFilesChanged([&] { ++calls; }, &calls);
    f.write("new.cpp");
    f.event(DirectoryNotifications::Added, "new.cpp");
    f.event(DirectoryNotifications::Added, "src/../new.cpp");
    f.drain();
    ASSERT_EQ(calls, 1);
    ASSERT_EQ(f.project->files().size(), 2u);
    f.project->updateCache();
    ASSERT_EQ(calls, 1);
}

TEST_CASE("removing files notifies once and absent removals are safe") {
    project_test::Fixture f;
    f.write("a.cpp");
    f.write("b.cpp");
    f.project->updateCache();
    int calls = 0;
    f.project->subscribeToFilesChanged([&] { ++calls; }, &calls);
    std::filesystem::remove(f.path("a.cpp"));
    f.event(DirectoryNotifications::Removed, "a.cpp");
    f.event(DirectoryNotifications::Removed, "a.cpp");
    f.event(DirectoryNotifications::Removed, "absent.cpp");
    f.drain();
    ASSERT_EQ(calls, 1);
    EXPECT_FALSE(f.contains("a.cpp"));
    EXPECT_TRUE(f.contains("b.cpp"));
    ASSERT_EQ(f.project->files().size(), 2u);
}

TEST_CASE("content changes do not publish file-list changes") {
    project_test::Fixture f;
    f.write("a.cpp", "before");
    f.project->updateCache();
    int calls = 0;
    f.project->subscribeToFilesChanged([&] { ++calls; }, &calls);
    f.write("a.cpp", "after");
    f.event(DirectoryNotifications::Changed, "a.cpp");
    f.drain();
    ASSERT_EQ(calls, 0);
    EXPECT_TRUE(f.contains("a.cpp"));
}

TEST_CASE("scans and additions use the same file selection rules") {
    project_test::Fixture f;
    f.write("src/keep.cpp");
    f.write("image.png");
    f.write("build/CMakeCache.txt");
    f.write("build/deep/generated.cpp");
    f.write("../outside.cpp");
    std::filesystem::create_directory(f.path("directory.cpp"));
    f.project->updateCache();
    ASSERT_EQ(f.project->files().size(), 2u);
    EXPECT_TRUE(f.contains("src/keep.cpp"));
    int calls = 0;
    f.project->subscribeToFilesChanged([&] { ++calls; }, &calls);
    for (const auto *path : {"image.png",
                             "build/deep/generated.cpp",
                             "directory.cpp",
                             "../outside.cpp",
                             "missing.cpp"}) {
        f.event(DirectoryNotifications::Added, path);
    }
    f.drain();
    ASSERT_EQ(calls, 0);
    ASSERT_EQ(f.project->files().size(), 2u);
}

TEST_CASE("incremental changes keep ordering and extension metadata current") {
    project_test::Fixture f;
    size_t observedCppCount = 0;
    f.project->subscribeToFilesChanged(
        [&] {
            observedCppCount = 0;
            for (const auto &[extension, count] : f.project->extensions()) {
                if (extension == ".cpp") {
                    observedCppCount = count;
                }
            }
        },
        &observedCppCount);
    f.write("z.cpp");
    f.write("a.cpp");
    f.event(DirectoryNotifications::Added, "z.cpp");
    f.event(DirectoryNotifications::Added, "a.cpp");
    f.drain();
    EXPECT_TRUE(
        std::is_sorted(f.project->files().begin(), f.project->files().end()));
    ASSERT_EQ(observedCppCount, 2u);
    f.event(DirectoryNotifications::Removed, "a.cpp");
    f.drain();
    ASSERT_EQ(observedCppCount, 1u);
}

TEST_CASE("unsubscribe prevents callbacks for queued changes") {
    project_test::Fixture f;
    int calls = 0;
    f.project->subscribeToFilesChanged([&] { ++calls; }, &calls);
    f.write("a.cpp");
    f.event(DirectoryNotifications::Added, "a.cpp");
    f.project->unsubscribeFromFilesChanged(&calls);
    f.drain();
    ASSERT_EQ(calls, 0);
    EXPECT_TRUE(f.contains("a.cpp"));
}

TEST_CASE("subscriptions may change during notification") {
    project_test::Fixture f;
    int first = 0;
    int removed = 0;
    int added = 0;
    f.project->subscribeToFilesChanged(
        [&] {
            ++first;
            f.project->unsubscribeFromFilesChanged(&first);
            f.project->unsubscribeFromFilesChanged(&removed);
            f.project->subscribeToFilesChanged([&] { ++added; }, &added);
        },
        &first);
    f.project->subscribeToFilesChanged([&] { ++removed; }, &removed);
    f.write("a.cpp");
    f.event(DirectoryNotifications::Added, "a.cpp");
    f.drain();
    ASSERT_EQ(first, 1);
    ASSERT_EQ(removed, 0);
    ASSERT_EQ(added, 0);
    f.write("b.cpp");
    f.event(DirectoryNotifications::Added, "b.cpp");
    f.drain();
    ASSERT_EQ(added, 1);
}

TEST_CASE("project destruction cancels queued and late directory events") {
    project_test::Fixture f;
    int calls = 0;
    f.project->subscribeToFilesChanged([&] { ++calls; }, &calls);
    f.write("a.cpp");
    f.event(DirectoryNotifications::Added, "a.cpp");
    const auto lateCallback = f.notify;
    f.project.reset();
    f.drain();
    lateCallback(DirectoryNotifications::Added, f.path("a.cpp"), {});
    f.drain();
    ASSERT_EQ(calls, 0);
}

TEST_CASE("settings content reloads without a file-list notification") {
    project_test::Fixture f;
    int calls = 0;
    f.project->subscribeToFilesChanged([&] { ++calls; }, &calls);
    f.write(".medit.json", R"({"build":"new build command"})");
    f.event(DirectoryNotifications::Changed, ".medit.json");
    f.drain();
    ASSERT_EQ(f.project->settings().buildCommand, "new build command");
    ASSERT_EQ(calls, 0);
}

TEST_CASE("changing project root refreshes files and keeps external settings") {
    project_test::Fixture f;
    f.write("old.cpp");
    f.write("../other/new.cpp");
    f.project->updateCache();
    int calls = 0;
    f.project->subscribeToFilesChanged([&] { ++calls; }, &calls);
    const auto newRoot = f.environment.directory / "other";
    f.write(".medit.json", "{\"root\":\"" + newRoot.string() + "\"}");
    f.event(DirectoryNotifications::Changed, ".medit.json");
    f.drain();
    ASSERT_EQ(calls, 1);
    EXPECT_FALSE(f.contains("old.cpp"));
    EXPECT_TRUE(f.contains("../other/new.cpp"));
    EXPECT_TRUE(f.contains(".medit.json"));
    ASSERT_EQ(f.project->files().size(), 2u);
}

TEST_SUIT_END

TEST_SUIT_BEGIN(SidebarProject)

TEST_CASE("sidebar observes project membership only after GUI queue delivery") {
    project_test::Fixture f;
    SideBar sidebar{nullptr};
    sidebar.project(*f.project);
    EXPECT_TRUE(sidebar.tree().reveal(f.path(".medit.json")));
    f.write("src/new.cpp");
    f.event(DirectoryNotifications::Added, "src/new.cpp");
    EXPECT_FALSE(sidebar.tree().reveal(f.path("src/new.cpp")));
    f.drain();
    EXPECT_TRUE(sidebar.tree().reveal(f.path("src/new.cpp")));
    f.event(DirectoryNotifications::Removed, "src/new.cpp");
    f.drain();
    EXPECT_FALSE(sidebar.tree().reveal(f.path("src/new.cpp")));
}

TEST_CASE("follow reveals the active file without changing sidebar focus") {
    project_test::Fixture f;
    f.write("src/one.cpp");
    f.write("src/two.cpp");
    f.project->updateCache();
    SideBar sidebar{nullptr};
    sidebar.project(*f.project);
    sidebar.activeFile(f.path("src/one.cpp"));
    EXPECT_FALSE(sidebar.focused());
    ASSERT_EQ(sidebar.tree().entries()[sidebar.tree().current()].path,
              f.path("src/one.cpp"));
    sidebar.toggleFollowActiveFile();
    sidebar.activeFile(f.path("src/two.cpp"));
    ASSERT_EQ(sidebar.tree().entries()[sidebar.tree().current()].path,
              f.path("src/one.cpp"));
    sidebar.toggleFollowActiveFile();
    ASSERT_EQ(sidebar.tree().entries()[sidebar.tree().current()].path,
              f.path("src/two.cpp"));
    EXPECT_FALSE(sidebar.focused());
    sidebar.tree().current(0);
    sidebar.activeFile(f.path("src/two.cpp"));
    ASSERT_EQ(sidebar.tree().current(), 0u);
}

TEST_CASE("pending follow can reveal a newly registered file without resetting "
          "later navigation") {
    project_test::Fixture f;
    SideBar sidebar{nullptr};
    sidebar.project(*f.project);
    sidebar.activeFile(f.path("src/new.cpp"));
    f.write("src/new.cpp");
    f.event(DirectoryNotifications::Added, "src/new.cpp");
    f.drain();
    ASSERT_EQ(sidebar.tree().entries()[sidebar.tree().current()].path,
              f.path("src/new.cpp"));
    sidebar.tree().current(0);
    f.write("src/other.cpp");
    f.event(DirectoryNotifications::Added, "src/other.cpp");
    f.drain();
    ASSERT_EQ(sidebar.tree().current(), 0u);
}

TEST_CASE("destroying a sidebar unsubscribes before queued project changes") {
    project_test::Fixture f;
    {
        SideBar sidebar{nullptr};
        sidebar.project(*f.project);
        f.write("new.cpp");
        f.event(DirectoryNotifications::Added, "new.cpp");
    }
    f.drain();
    EXPECT_TRUE(f.contains("new.cpp"));
}

TEST_SUIT_END
