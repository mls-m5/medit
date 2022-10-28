#pragma once

#include "mls-unit-test/mock.h"
#include "script/iscope.h"

class MockScope : public IScope {
public:
    MOCK_METHOD0(Editor &, editor, (), override);
    MOCK_METHOD0(Editor &, console, (), override);
    MOCK_METHOD0(KeyEvent, key, (), const override);
    MOCK_METHOD0(IScope &, parent, (), override);
    MOCK_METHOD0(IScope &, root, (), override);
    MOCK_METHOD0(Project &, project, (), override);
    MOCK_METHOD0(Context &, context, (), override);
    MOCK_METHOD0(Registers &, registers, (), override);
    MOCK_CONST_METHOD0(IScope &, root, (), override);
    MOCK_METHOD2(void,
                 addCommand,
                 (std::string,
                  std::function<void(std::shared_ptr<IScope>)>),
                 override);
    MOCK_METHOD1(void, showConsole, (bool shown), override);
    MOCK_METHOD1(bool, run, (const Command &command), override);
    MOCK_METHOD1(Action *, findAction, (const std::string &name), override);

    MOCK_METHOD2(void, set, (std::string name, Variable variable), override);
    MOCK_CONST_METHOD1(std::optional<Variable>,
                       get,
                       (std::string name),
                       override);
};
