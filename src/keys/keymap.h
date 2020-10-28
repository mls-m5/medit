#pragma once

#include "keyevent.h"
#include <vector>

struct KeyMatch {
    KeyEvent event;
};

class KeyMap {
    std::string defaultAction;
    std::vector<std::pair<KeyMatch, std::string>> _map;

public:
    KeyMap() = default;
    KeyMap(const KeyMap &) = default;
    KeyMap(KeyMap &&) = default;
    KeyMap &operator=(const KeyMap &) = default;
    KeyMap &operator=(KeyMap &&) = default;

    KeyMap(std::vector<std::pair<KeyMatch, std::string>> map)
        : _map(std::move(map)) {}

    std::string find(const KeyEvent &event) const;
};
