#pragma once

#include "files/directorynotifications.h"
#include "mls-unit-test/mock.h"

class MockDirectoryNotifications : public DirectoryNotifications {
public:
    MOCK_METHOD1(void, path, (std::filesystem::path), override);
    MOCK_METHOD2(void, subscribe, (CallbackT, void *ref), override);
    MOCK_METHOD1(void, unsubcribe, (void *ref), override);
};
