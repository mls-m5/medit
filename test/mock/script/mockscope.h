#pragma once

// #include "mls-unit-test/mock.h"
// #include "mockenvironment.h"
// #include "script/ienvironment.h"
// #include <string_view>

// class MockScope : public IEnvironment {
// public:
//     MOCK_METHOD0(Editor &, editor, (), override);
//     MOCK_METHOD0(IEnvironment &, parent, (), override);
//     MOCK_METHOD0(IEnvironment &, root, (), override);
//     MOCK_CONST_METHOD0(IEnvironment &, root, (), override);
//     MOCK_METHOD2(void,
//                  addCommand,
//                  (std::string,
//                   std::function<void(std::shared_ptr<IEnvironment>)>),
//                  override);
//     MOCK_METHOD1(bool, run, (const Command &command), override);
//     MOCK_METHOD1(Action *, findAction, (const std::string &name), override);

//    MOCK_METHOD2(void, set, (std::string name, Variable variable), override);
//    MOCK_CONST_METHOD1(std::optional<Variable>,
//                       get,
//                       (std::string name),
//                       override);

//    IEnvironment &env() override {
//        return environment;
//    }

//    MockEnvironment environment;
//};
