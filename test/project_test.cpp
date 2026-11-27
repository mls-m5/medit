
#include "core/jobqueue.h"
#include "files/project.h"
#include "mls-unit-test/unittest.h"
#include "mock/files/mockdirectorynotifications.h"
#include <algorithm>
#include <filesystem>

const std::filesystem::path path = "test/res/.medit.json";
const std::filesystem::path root = "test/res";

TEST_SUIT_BEGIN(Project)

TEST_CASE("load") {
    MockDirectoryNotifications dirNotifications;

    dirNotifications.mock_subscribe_2.expectNum(1);

    auto jobQueue = JobQueue{};

    auto project = Project{path, dirNotifications, jobQueue};

    project.updateCache();

    ASSERT_EQ(project.settings().root, std::filesystem::absolute(root));

    ASSERT_EQ(project.settings().buildCommand, "matmake");
}

TEST_CASE("removing a path that is not cached keeps the other files") {
    MockDirectoryNotifications dirNotifications;
    DirectoryNotifications::CallbackT notify;
    dirNotifications.mock_subscribe_2.expectNum(1);
    dirNotifications.mock_subscribe_2.onCall(
        [&](auto callback, auto) { notify = callback; });

    auto jobQueue = JobQueue{};
    auto project = Project{path, dirNotifications, jobQueue};
    project.updateCache();
    const auto before = project.files();
    ASSERT_GT(before.size(), 1u);

    notify(DirectoryNotifications::Removed,
           std::filesystem::absolute(root) / "not-in-project.cpp",
           {});
    jobQueue.work(false);
    ASSERT_EQ(project.files().size(), before.size());
    ASSERT_EQ(project.files().back(), before.back());

    notify(DirectoryNotifications::Removed, before.front(), {});
    jobQueue.work(false);
    ASSERT_EQ(project.files().size(), before.size() - 1);
    ASSERT_EQ(std::count(project.files().begin(),
                         project.files().end(),
                         before.front()),
              0);
    ASSERT_EQ(project.files().back(), before.back());
}

TEST_SUIT_END
