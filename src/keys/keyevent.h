#pragma once

#include "text/utf8char.h"

enum class Key {
    Unknown,
    Any,
    Text,
    KeyCombination,
    Up,
    Down,
    Left,
    Right,
    Backspace,
    Delete,
    Return,
    Space,
    Escape,
    Cancel,
};

enum class Modifiers {
    None = 0,
    Ctrl = 1 << 0,
    Alt = 1 << 1,
};

class KeyEvent {
public:
    KeyEvent() = default;
    KeyEvent(const KeyEvent &) = default;
    KeyEvent(KeyEvent &&) = default;
    KeyEvent &operator=(const KeyEvent &) = default;
    KeyEvent &operator=(KeyEvent &&) = default;

    KeyEvent(Key key,
             Utf8Char symbol = {},
             Modifiers modifiers = Modifiers::None,
             bool state = true)
        : key(key), symbol(symbol), modifiers(modifiers), state(state) {}

    Key key = Key::Unknown;
    Utf8Char symbol = {};
    Modifiers modifiers = Modifiers::None;
    bool state = true;

    constexpr friend bool operator==(const KeyEvent &a, const KeyEvent &b) {
        return a.key == b.key && a.modifiers == b.modifiers &&
               a.state == b.state && a.symbol == b.symbol;
    }
};
