
#include "keymap.h"
#include <algorithm>

const CommandT &KeyMap::find(const KeyEvent &event) const {
    auto f = std::find_if(_map.begin(), _map.end(), [&event](auto &e) {
        return event == e.first.event;
    });

    if (f != _map.end()) {
        return f->second;
    }

    return _defaultAction;
}
