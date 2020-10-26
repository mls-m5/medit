#pragma once

class KeyEvent {
public:
    int key = 0;
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
