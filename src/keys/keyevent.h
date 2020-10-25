#pragma once

struct KeyEvent {
    int key = 0;
    int modifiers = 0;
    bool state = true;

    friend bool operator<(const KeyEvent &a, const KeyEvent &b) {
        return a.key < b.key && a.modifiers < b.modifiers && a.state < b.state;
    }

    friend bool operator==(const KeyEvent &a, const KeyEvent &b) {
        return a.key == b.key && a.modifiers == b.modifiers &&
               a.state == b.state;
    }
};
