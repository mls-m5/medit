#include "mode.h"

#include "script/standardcommands.h"

Mode::Mode(std::string name,
           KeyMap map,
           CursorStyle cursorStyle,
           std::shared_ptr<IMode> parent,
           BufferKeyMap bufferMap)
    : _name(std::move(name)), _keyMap(std::move(map)),
      _bufferMap(std::move(bufferMap)), _parent(std::move(parent)),
      _cursorStyle(cursorStyle) {}

bool Mode::keyPress(std::shared_ptr<IEnvironment> env) {
    auto lock = shared_from_this();
    const auto key = env->key();
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
            env->run(block);
            _buffer.clear();
            return true;
        }
    }
    _buffer.clear();

    if (action) {
        env->run(action);
        return true;
    }
    return false;
}
