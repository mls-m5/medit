#include "mode.h"

#include "script/ienvironment.h"

Mode::Mode(std::string name, KeyMap map, std::shared_ptr<IMode> parent)
    : _name(std::move(name))
    , _keyMap(std::move(map))
    , _parent(std::move(parent)) {}

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

        auto intRep = static_cast<uint32_t>(key.symbol);
        if (_shouldEnableNumbers && (intRep >= '0' && intRep <= '9')) {
            auto number = intRep - '0';
            if (_repetitions) {
                _repetitions = _repetitions * 10 + number;
            }
            else {
                _repetitions = number;
            }

            if (_repetitions > 99999) {
                _repetitions = 99999;
            }

            env->statusMessage(
                FString{"repeat " + std::to_string(_repetitions)});
            return true;
        }
        else if (m.first == BufferKeyMap::PartialMatch) {
            _buffer = bufferText;
            return true;
        }
        else if (m.first == BufferKeyMap::Match) {
            const auto &block = *m.second;
            block(env);
            _buffer.clear();
            return true;
        }
    }
    _buffer.clear();

    if (action) {
        action(env);
        return true;
    }
    return false;
}
