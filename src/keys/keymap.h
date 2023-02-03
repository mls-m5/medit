#pragma once

#include "event.h"
#include "script/command.h"
#include <vector>

struct KeyMatch {
    KeyMatch() = default;
    KeyMatch(const KeyMatch &) = default;
    KeyMatch(KeyMatch &&) = default;
    KeyMatch &operator=(const KeyMatch &) = default;
    KeyMatch &operator=(KeyMatch &&) = default;

    KeyMatch(KeyEvent key) : event(key) {}
    KeyMatch(std::string_view str) {
        event = {};
        event.key = Key::Text;
        event.symbol = str.data();
    }

    KeyEvent event;
};

class KeyMap {
    CommandBlock _defaultAction;
    std::vector<std::pair<KeyMatch, CommandBlock>> _map;

public:
    KeyMap() = default;
    KeyMap(const KeyMap &) = delete;
    KeyMap(KeyMap &&) = default;
    KeyMap &operator=(const KeyMap &) = delete;
    KeyMap &operator=(KeyMap &&) = default;

    KeyMap(std::vector<std::pair<KeyMatch, CommandBlock>> map)
        : _map(std::move(map)) {}

    const CommandBlock &find(const KeyEvent &event) const;

    void defaultAction(CommandBlock action) {
        _defaultAction = std::move(action);
    }

    CommandBlock defaultAction() const {
        return _defaultAction;
    }
};
