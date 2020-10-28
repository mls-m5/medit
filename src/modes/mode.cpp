

#include "mode.h"
#include "script/matscript.h"

Mode::Mode(std::string name, KeyMap map)
    : _name(std::move(name)), _keyMap(std::move(map)) {}

void Mode::keyPress(IEnvironment &env) {
    auto action = _keyMap.find(env.key());
    if (!action.empty()) {
        run(action, env);
    }
    else {
        run(_keyMap.defaultAction(), env);
    }
}
