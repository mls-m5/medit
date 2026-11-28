#include "core/jobqueue.h"
#include "files/memoryfilesystem.h"
#include "files/project.h"
#include "mls-unit-test/unittest.h"
#include <algorithm>
#include <filesystem>

namespace project_test {

const std::filesystem::path path = "test/res/.medit.json";
const std::filesystem::path root = std::filesystem::absolute("test/res");

struct Fixture {
    MemoryFileSystem fs;
    JobQueue jobs;
    JobQueue gui;
    Project project{path, fs, jobs, gui};

    void runQueues() {
        for (int i = 0; i < 10; ++i) {
            jobs.work(false);
            gui.work(false);
        }
    }
};

} // namespace project_test

TEST_SUIT_BEGIN(Project)

TEST_CASE("load") {
    project_test::Fixture f;
    f.project.refreshNow();
    ASSERT_EQ(f.project.settings().root, project_test::root);
    ASSERT_EQ(f.project.settings().buildCommand, "matmake");
}

TEST_CASE("the directory source skips build directories and .git") {
    using project_test::root;
    project_test::Fixture f;
    f.fs.write(root / "b.cpp", "");
    f.fs.write(root / "src" / "a.h", "");
    f.fs.write(root / "notes.unknownextension", "");
    f.fs.write(root / "build" / "CMakeCache.txt", "");
    f.fs.write(root / "build" / "generated.cpp", "");
    f.fs.write(root / ".git" / "config.json", "");
    f.project.refreshNow();

    // Sorted, with the settings file always included.
    EXPECT_TRUE((f.project.files() ==
                 std::vector<std::filesystem::path>{
                     root / ".medit.json", root / "b.cpp", root / "src/a.h"}));
    EXPECT_FALSE(f.project.projectFiles().truncated);
    EXPECT_TRUE(f.project.getProjectLanguage() == Project::Cpp);

    f.project.fileLimit(1);
    f.project.refreshNow();
    EXPECT_TRUE(f.project.projectFiles().truncated);
}

TEST_CASE("refreshes run on the job queue and notify only on changes") {
    using project_test::root;
    project_test::Fixture f;
    f.fs.write(root / "a.cpp", "");
    f.project.refreshNow();

    auto calls = 0;
    auto subscription = f.project.subscribeToFiles([&] { ++calls; });

    f.fs.write(root / "b.cpp", "");
    f.project.refresh();
    // Nothing changes until the queues have run.
    ASSERT_EQ(f.project.files().size(), 2u);
    f.runQueues();
    ASSERT_EQ(f.project.files().size(), 3u);
    ASSERT_EQ(calls, 1);

    // Same files: no notification.
    f.project.refresh();
    f.runQueues();
    ASSERT_EQ(calls, 1);

    // The subscription ends with its handle.
    subscription.reset();
    f.fs.write(root / "c.cpp", "");
    f.project.refresh();
    f.runQueues();
    ASSERT_EQ(f.project.files().size(), 4u);
    ASSERT_EQ(calls, 1);
}

TEST_CASE("refreshes while one runs are coalesced into one more") {
    using project_test::root;
    project_test::Fixture f;
    f.project.refreshNow();
    auto calls = 0;
    auto subscription = f.project.subscribeToFiles([&] { ++calls; });

    f.fs.write(root / "a.cpp", "");
    f.project.refresh();
    f.project.refresh(); // Pending.
    f.project.refresh(); // Same pending one.

    f.jobs.work(false); // The first listing.
    f.fs.write(root / "b.cpp", "");
    f.gui.work(false); // Applies it and starts the pending one.
    ASSERT_EQ(calls, 1);
    f.jobs.work(false);
    f.gui.work(false);
    ASSERT_EQ(calls, 2);
    ASSERT_EQ(f.project.files().size(), 3u);
    // Nothing more was queued.
    f.runQueues();
    ASSERT_EQ(calls, 2);
}

TEST_CASE("a listing that finishes after the project is gone is dropped") {
    MemoryFileSystem fs;
    JobQueue jobs;
    JobQueue gui;
    {
        auto project = Project{project_test::path, fs, jobs, gui};
        project.refresh();
        jobs.work(false);
    }
    gui.work(false); // Must not touch the destroyed project.
}

TEST_SUIT_END
