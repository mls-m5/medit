#pragma once

#include "keys/keymap.h"
#include "modes/imode.h"
#include <string>

class Mode : public IMode {
    std::string _name;
    KeyMap _keyMap;

public:
    Mode(std::string name, KeyMap map);

    void keyPress(const KeyEvent &, Editor &) override;

    std::string_view name() override {
        return _name;
    }
};
