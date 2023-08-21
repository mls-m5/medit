#include "keys/event.h"
#include "core/archive.h"
#include "text/fstring.h"
#include <variant>

KeyEvent KeyEvent::translate(std::string name) {

    if (name.size() == 1) {
        return {Key::Text, name.at(0)};
    }

#define E(x)                                                                   \
    if (name == #x) {                                                          \
        return Key::x;                                                         \
    }

    // E(Unknown);
    // E(Any);
    // E(Text);
    // E(KeyCombination);
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

void KeyEvent::visit(Archive &arch) {
    arch("key", key);
    arch("symbol", symbol);
    arch("modifiers", modifiers);
    arch("state", state);
}

void MouseDownEvent::visit(Archive &arch) {
    ARCH_PAIR(button);
    ARCH_PAIR(x);
    ARCH_PAIR(y);
}

void MouseMoveEvent::visit(Archive &arch) {
    ARCH_PAIR(button);
    ARCH_PAIR(x);
    ARCH_PAIR(y);
}

void NullEvent::visit(Archive &arch) {}

void PasteEvent::visit(Archive &arch) {
    ARCH_PAIR(text);
}

void ResizeEvent::visit(Archive &arch) {
    ARCH_PAIR(width);
    ARCH_PAIR(height);
}

void load(Archive &arch, Event &e) {
    size_t t = 0;
    arch("event_type", t);
    switch (t) {
    case 0:
        e = NullEvent{};
        break;
    case 1: {
        auto e = KeyEvent{};
        break;
    }
    case 2: {
        auto pe = MouseDownEvent{};
        break;
    }
    case 3: {
        auto pe = MouseMoveEvent{};
        break;
    }
    case 4: {
        auto pe = PasteEvent{};
        break;
    }
    case 5: {
        auto pe = ResizeEvent{};
        break;
    }
    }

    arch.beginChild("event");
    std::visit([&arch](auto &e) { e.visit(arch); }, e);
    arch.endChild();
}

void save(Archive &arch, Event &e) {
    auto t = e.index();

    arch("event_type", t);

    arch.beginChild("event");
    std::visit([&arch](auto &e) { e.visit(arch); }, e);
    arch.endChild();
}
