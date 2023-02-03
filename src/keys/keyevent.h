#pragma once

#include "text/utf8char.h"

enum class Key {
    Unknown,
    None,
    Any,
    Text,
    KeyCombination,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    PageUp,
    PageDown,
    Backspace,
    Delete,
    //    Return, never used
    Space,
    Escape,
    Cancel,
    Tab,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    Resize, // Window resize event
    Quit,
};

enum class Modifiers {
    None = 0,
    Ctrl = 1 << 0,
    Alt = 1 << 1,
};

class KeyEvent {
public:
    constexpr KeyEvent() = default;
    constexpr KeyEvent(const KeyEvent &) = default;
    constexpr KeyEvent(KeyEvent &&) = default;
    constexpr KeyEvent &operator=(const KeyEvent &) = default;
    constexpr KeyEvent &operator=(KeyEvent &&) = default;

    constexpr KeyEvent(Key key,
                       Utf8Char symbol = {},
                       Modifiers modifiers = Modifiers::None,
                       bool state = true)
        : key(key)
        , symbol(symbol)
        , modifiers(modifiers)
        , state(state) {}

    constexpr friend bool operator==(const KeyEvent &a, const KeyEvent &b) {
        return a.key == b.key && a.modifiers == b.modifiers &&
               a.state == b.state && a.symbol == b.symbol;
    }

    constexpr friend bool operator!=(const KeyEvent &a, const KeyEvent &b) {
        return !(a == b);
    }

    Key key = Key::Unknown;
    Utf8Char symbol = {};
    Modifiers modifiers = Modifiers::None;
    bool state = true;
};
