#include "keys/event.h"
#include "core/archive.h"
#include "text/fstring.h"
#include <variant>

std::string KeyEvent::description() const {
    auto name = std::string{};
    switch (key) {
    case Key::Up:
        name = "Up";
        break;
    case Key::Down:
        name = "Down";
        break;
    case Key::Left:
        name = "Left";
        break;
    case Key::Right:
        name = "Right";
        break;
    case Key::Home:
        name = "Home";
        break;
    case Key::End:
        name = "End";
        break;
    case Key::PageUp:
        name = "PageUp";
        break;
    case Key::PageDown:
        name = "PageDown";
        break;
    case Key::Backspace:
        name = "Backspace";
        break;
    case Key::Delete:
        name = "Delete";
        break;
    case Key::Space:
        name = "Space";
        break;
    case Key::Escape:
        name = "Escape";
        break;
    case Key::Tab:
        name = "Tab";
        break;
    case Key::F1:
    case Key::F2:
    case Key::F3:
    case Key::F4:
    case Key::F5:
    case Key::F6:
    case Key::F7:
    case Key::F8:
    case Key::F9:
    case Key::F10:
    case Key::F11:
    case Key::F12:
        name = "F" + std::to_string(static_cast<int>(key) -
                                    static_cast<int>(Key::F1) + 1);
        break;
    case Key::Text:
    case Key::KeyCombination:
        if (symbol == '\t') {
            name = "Tab";
        }
        else if (symbol == ' ') {
            name = "Space";
        }
        else if (symbol == '\n') {
            name = "Enter";
        }
        else {
            name = std::string{std::string_view{symbol}};
        }
        break;
    default:
        return {};
    }

    const auto has = [this](Modifiers modifier) {
        return (static_cast<int>(modifiers) & static_cast<int>(modifier)) != 0;
    };
    if (has(Modifiers::Alt)) {
        name = "Alt+" + name;
    }
    if (has(Modifiers::Ctrl)) {
        name = "Ctrl+" + name;
    }
    return name;
}

KeyEvent KeyEvent::translate(std::string name) {

    if (name.size() == 1) {
        return {Key::Text, name.at(0)};
    }

#define E(x)                                                                   \
    if (name == #x) {                                                          \
        return Key::x;                                                         \
    }

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
        e = KeyEvent{};
        break;
    }
    case 2: {
        e = MouseDownEvent{};
        break;
    }
    case 3: {
        e = MouseMoveEvent{};
        break;
    }
    case 4: {
        e = PasteEvent{};
        break;
    }
    case 5: {
        e = ResizeEvent{};
        break;
    }
    }

    arch.beginChild("event");
    std::visit(
        [&arch](auto &e) {
            e.visit(arch); //
        },
        e);
    arch.endChild();
}

void save(Archive &arch, Event &e) {
    auto t = e.index();

    arch("event_type", t);

    arch.beginChild("event");
    std::visit([&arch](auto &e) { e.visit(arch); }, e);
    arch.endChild();
}
