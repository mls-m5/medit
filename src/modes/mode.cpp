

#include "mode.h"

Mode::Mode(std::string name, KeyMap map) {
    _name = name;
    _keyMap = map;
}

void Mode::keyPress(const KeyEvent event, Editor &) {
    auto action = _keyMap.find(event)
}
