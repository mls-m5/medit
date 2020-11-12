#pragma once

#include "keys/bufferkeymap.h"
#include "keys/keymap.h"
#include "modes/imode.h"
#include <memory>
#include <string>

class Mode : public IMode {

    std::string _name;
    KeyMap _keyMap;
    std::string _commandBuffer;
    bool _bufferMode = false;
    //    std::vector<KeyEvent> _bufferTriggers;
    FString _buffer;
    BufferKeyMap _bufferMap;
    std::unique_ptr<IMode> _parent;

public:
    Mode(std::string name,
         KeyMap map,
         std::unique_ptr<IMode> parent = {},
         BufferKeyMap bufferMap = {});

    bool keyPress(IEnvironment &env) override;

    std::string_view name() override {
        return _name;
    }
};
