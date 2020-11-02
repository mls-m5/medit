#pragma once

#include "keys/keymap.h"
#include "modes/imode.h"

#include <memory>
#include <string>

class Mode : public IMode {
    std::string _name;
    KeyMap _keyMap;
    std::unique_ptr<IMode> _parent;

public:
    Mode(std::string name, KeyMap map, std::unique_ptr<IMode> parent = {});

    bool keyPress(IEnvironment &env, bool useDefault = true) override;

    std::string_view name() override {
        return _name;
    }
};
