
#include "core/jobqueue.h"
#include "files/project.h"
#include "mls-unit-test/unittest.h"
#include "mock/files/mockdirectorynotifications.h"
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

TEST_SUIT_END
