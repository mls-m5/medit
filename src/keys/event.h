#pragma once

#include "nlohmann/json.hpp"
#include "text/utf8char.h"
#include <functional>
#include <variant>

enum class Key {
    Unknown,
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

    bool hasModifiers() const {
        return modifiers != Modifiers::None;
    }

    /// Create a key event from string representation
    static KeyEvent translate(std::string name);

    Key key = Key::Unknown;
    Utf8Char symbol = {};
    Modifiers modifiers = Modifiers::None;
    bool state = true;
};

struct MouseDownEvent {
public:
    int button = 1;
    int x = -1;
    int y = -1;
};

struct MouseMoveEvent {
public:
    int button = 1;
    int x = -1;
    int y = -1;
};

struct NullEvent {};

struct PasteEvent {
    std::string text;
};

struct ResizeEvent {
    size_t width;
    size_t height;
};

using Event = std::variant<NullEvent,
                           KeyEvent,
                           MouseDownEvent,
                           MouseMoveEvent,
                           PasteEvent,
                           ResizeEvent>;
