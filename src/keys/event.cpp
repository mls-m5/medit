#include "keys/event.h"
#include "text/fstring.h"

KeyEvent KeyEvent::translate(std::string name) {

    if (name.size() == 1) {
        return {Key::Text, name.at(0)};
    }

#define E(x)                                                                   \
    if (name == #x) {                                                          \
        return Key::x;                                                         \
    }

    E(Unknown);
    E(Any);
    E(Text);
    E(KeyCombination);
    E(Up);
    E(Down);
    E(Left);
    E(Right);
    E(Home);
    E(End);
    E(PageUp);
    E(PageDown);
    E(Backspace);
    E(Delete);
    E(Space);
    E(Escape);
    E(Cancel);
    E(Tab);
    E(F1);
    E(F2);
    E(F3);
    E(F4);
    E(F5);
    E(F6);
    E(F7);
    E(F8);
    E(F9);
    E(F10);
    E(F11);
    E(F12);
    E(Quit);

#undef E

    auto fname = FString{name};
    return KeyEvent{};
}
