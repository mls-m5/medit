#include "mode.h"

#include "script/standardcommands.h"

Mode::Mode(std::string name,
           KeyMap map,
           CursorStyle cursorStyle,
           std::unique_ptr<IMode> parent,
           BufferKeyMap bufferMap)
    : _name(std::move(name)), _keyMap(std::move(map)),
      _bufferMap(std::move(bufferMap)), _parent(std::move(parent)),
      _cursorStyle(cursorStyle) {}

bool Mode::keyPress(IEnvironment &env) {
    const auto key = env.key();
    const auto &action = _keyMap.find(key);
    if (_parent && _parent->keyPress(env)) {
        return true;
    }

    if (key.key == Key::Text) {
        auto bufferText = _buffer;
        bufferText.emplace_back(key.symbol);
        auto m = _bufferMap.match(bufferText);

        if (m.first == BufferKeyMap::PartialMatch) {
            _buffer = bufferText;
            return true;
        }
        else if (m.first == BufferKeyMap::Match) {
            const auto &block = *m.second;
            env.run(block);
            _buffer.clear();
            return true;
        }
    }
    _buffer.clear();

    if (action) {
        env.run(action);
        return true;
    }
    //    else if (useDefault) {
    //        env.run(_keyMap.defaultAction());
    //        return true;
    //    }
    return false;
}
