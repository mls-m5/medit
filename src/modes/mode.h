#pragma once

#include "keys/bufferkeymap.h"
#include "keys/keymap.h"
#include "modes/imode.h"
#include <memory>
#include <string>

class Mode : public IMode {

    std::string _name;
    KeyMap _keyMap;
    std::unique_ptr<IMode> _parent;
    std::string _commandBuffer;
    bool _bufferMode = false;
    std::vector<Utf8Char> _bufferTriggers;
    BufferKeyMap _bufferMap;

public:
    Mode(std::string name,
         KeyMap map,
         std::unique_ptr<IMode> parent = {},
         BufferKeyMap bufferMap = {});

    bool keyPress(IEnvironment &env, bool useDefault = true) override;

    std::string_view name() override {
        return _name;
    }
};
