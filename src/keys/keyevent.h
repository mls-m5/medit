#pragma once

#include "text/utf8char.h"

enum class Key {
    Unknown,
    Text,
    Up,
    Down,
    Left,
    Right,
    Backspace,
    Return,
};

class KeyEvent {
public:
    Key key = Key::Unknown;
    Utf8Char symbol = {};
    int modifiers = 0;
    bool state = true;

    constexpr friend bool operator<(const KeyEvent &a, const KeyEvent &b) {
        return a.key < b.key && a.modifiers < b.modifiers && a.state < b.state;
    }

    constexpr friend bool operator==(const KeyEvent &a, const KeyEvent &b) {
        return a.key == b.key && a.modifiers == b.modifiers &&
               a.state == b.state;
    }
};
