#include "core/jobqueue.h"
#include "core/timer.h"
#include "files/gitstatus.h"
#include "files/project.h"
#include "helpers/temporaryproject.h"
#include "linux/inotify.h"
#include "mls-unit-test/unittest.h"
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <mutex>

namespace git_status_test {
class GuiQueue : public JobQueue {
    std::mutex mutex;
    std::condition_variable ready;
    size_t submitted = 0;

public:
    void addTask(std::function<void()> task) override {
        JobQueue::addTask(std::move(task));
        {
            auto lock = std::lock_guard{mutex};
            ++submitted;
        }
        ready.notify_all();
    }
    template <class Predicate>
    void until(Predicate predicate, const char *description) {
        const auto deadline =
            std::chrono::steady_clock::now() + std::chrono::seconds{5};
        while (true) {
            size_t before;
            {
                auto lock = std::lock_guard{mutex};
                before = submitted;
            }
            work(false);
            if (predicate())
                return;
            auto lock = std::unique_lock{mutex};
            if (!ready.wait_until(
                    lock, deadline, [&] { return submitted != before; }))
                throw std::runtime_error(description);
        }
    }
};

void run(const std::filesystem::path &root, std::string_view arguments) {
    const auto command = "git -C '" + root.string() + "' " +
                         std::string{arguments} + " >/dev/null 2>&1";
    if (std::system(command.c_str()) != 0)
        throw std::runtime_error("Git command failed: " + command);
}
void commit(test_helpers::TemporaryProject &project) {
    run(project.root, "init -q");
    run(project.root, "add .");
    run(project.root,
        "-c user.name=Test -c user.email=test@example.org commit -qm initial");
}
} // namespace git_status_test

TEST_SUIT_BEGIN(GitStatus)

TEST_CASE("one Git snapshot includes modified staged and untracked files") {
    test_helpers::TemporaryProject project;
    project.write("clean.cpp", "clean\n");
    project.write("staged.cpp", "old\n");
    project.write("renamed.cpp", "old name\n");
    project.write(".gitignore", "ignored.cpp\n");
    git_status_test::commit(project);
    project.write("staged.cpp", "staged\n");
    git_status_test::run(project.root, "add staged.cpp");
    std::filesystem::rename(project.path("renamed.cpp"),
                            project.path("renamed with spaces.cpp"));
    git_status_test::run(project.root, "add -A");
    project.write("untracked with spaces.cpp", "new\n");
    project.write("ignored.cpp", "ignored\n");
    auto changed = changedGitFiles(project.root);
    EXPECT_TRUE(changed.contains(project.path("staged.cpp")));
    EXPECT_TRUE(changed.contains(project.path("renamed with spaces.cpp")));
    EXPECT_TRUE(changed.contains(project.path("untracked with spaces.cpp")));
    EXPECT_FALSE(changed.contains(project.path("clean.cpp")));
    EXPECT_FALSE(changed.contains(project.path("ignored.cpp")));
    EXPECT_FALSE(changed.contains(project.path("renamed.cpp")));
    project.write("clean.cpp", "modified\n");
    changed = changedGitFiles(project.root);
    EXPECT_TRUE(changed.contains(project.path("clean.cpp")));
    EXPECT_TRUE(changedGitFiles(project.path("not-a-repository")).empty());
}

#ifdef MEDIT_USING_LINUX
TEST_CASE(
    "project caches Git status on the GUI queue and notifies on changes") {
    test_helpers::TemporaryProject project;
    project.write("file.cpp", "clean\n");
    git_status_test::commit(project);
    git_status_test::GuiQueue gui;
    JobQueue background;
    Timer timer;
    auto notifications = createDirectoryNotifications(gui);
    Project model{
        project.path(".medit.json"), *notifications, gui, &background, &timer};
    model.updateCache();
    int changes = 0;
    model.subscribeToGitStatusChanged([&] { ++changes; }, &changes);
    background.start();
    timer.start();
    model.enableGitStatus();
    project.write("file.cpp", "modified\n");
    gui.until([&] { return model.gitChanged(project.path("file.cpp")); },
              "Git snapshot includes modified file");
    EXPECT_TRUE(changes > 0);
    project.write("file.cpp", "clean\n");
    gui.until([&] { return !model.gitChanged(project.path("file.cpp")); },
              "Git status refresh after file change");
    EXPECT_FALSE(model.gitChanged(project.path("absent.cpp")));
    model.unsubscribeFromGitStatusChanged(&changes);
    notifications->path({});
    timer.stop();
    background.stop();
}
#endif

TEST_SUIT_END
