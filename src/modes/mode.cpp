#include "mode.h"

#include "script/matscript.h"

Mode::Mode(std::string name, KeyMap map, std::unique_ptr<IMode> parent)
    : _name(std::move(name)), _keyMap(std::move(map)),
      _parent(std::move(parent)) {}

bool Mode::keyPress(IEnvironment &env, bool useDefault) {
    auto action = _keyMap.find(env.key());
    if (_parent && _parent->keyPress(env, false)) {
        return true;
    }
    if (!action.empty()) {
        run(action, env);
        return true;
    }
    else if (useDefault) {
        run(_keyMap.defaultAction(), env);
        return true;
    }
    return false;
}
