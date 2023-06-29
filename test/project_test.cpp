
#include "files/filesystem.h"
#include "files/project.h"
#include "mls-unit-test/unittest.h"
#include "mock/files/mockdirectorynotifications.h"

const filesystem::path path = "test/res/.medit.json";
const filesystem::path root = "test/res";

TEST_SUIT_BEGIN

TEST_CASE("load") {
    MockDirectoryNotifications dirNotifications;

    dirNotifications.mock_subscribe_2.expectNum(1);

    auto project = Project{dirNotifications};

    project.updateCache(path);

    ASSERT_EQ(project.settings().root, filesystem::absolute(root));

    ASSERT_EQ(project.settings().buildCommand, "matmake");
}

TEST_SUIT_END
