#include "mode.h"

#include "script/standardcommands.h"

Mode::Mode(std::string name,
           KeyMap map,
           std::unique_ptr<IMode> parent,
           BufferKeyMap bufferMap)
    : _name(std::move(name)), _keyMap(std::move(map)),
      /* _bufferMap(std::move(bufferMap)),*/ _parent(std::move(parent)) {}

bool Mode::keyPress(IEnvironment &env, bool useDefault) {
    auto action = _keyMap.find(env.key());
    if (_parent && _parent->keyPress(env, false)) {
        return true;
    }
    if (action) {
        env.run(action);
        //        run(action, env);
        return true;
    }
    else if (useDefault) {
        env.run(_keyMap.defaultAction());
        //        run(_keyMap.defaultAction(), env);
        return true;
    }
    return false;
}
