
#include "keymap.h"

std::string KeyMap::find(const KeyEvent &event) const {
    auto f = std::find_if(
        _map.begin(), _map.end(), [&event](auto &e) { return event == e; });

    if (f != _map.end()) {
        return f->second;
    }

    return defaultAction;
}
