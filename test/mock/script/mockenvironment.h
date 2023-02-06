#pragma once

#include "keys/event.h"
#include "meditfwd.h"
#include "mls-unit-test/mock.h"
#include "script/ienvironment.h"

class MockEnvironment : public IEnvironment {
public:
    MOCK_METHOD0(KeyEvent, key, (), const override);
    MOCK_METHOD0(Project &, project, (), override);
    MOCK_METHOD0(Context &, context, (), override);
    MOCK_METHOD0(Registers &, registers, (), override);
    MOCK_METHOD1(void, showConsole, (bool shown), override);
    MOCK_METHOD0(Editor &, console, (), override);
    MOCK_METHOD0(CoreEnvironment &, core, (), override);
    MOCK_METHOD0(MainWindow &, mainWindow, (), override);
};
