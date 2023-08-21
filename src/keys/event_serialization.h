#pragma once

#include "core/archive.h"
#include "keys/event.h"
#include "nlohmann/json.hpp"
#include "text/utf8char_serialization.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeyEvent, key, symbol, modifiers, state);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MouseDownEvent, button, x, y);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MouseMoveEvent, button, x, y);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PasteEvent, text);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ResizeEvent, width, height);

inline void to_json(nlohmann::json &j, const Event &e) {
    j["event_type"] = e.index();
    std::visit([&j](auto &&e) { j["event"] = e; }, e);
}

// inline void KeyEvent::visit(Archive &arch) {
//     arch("key", key);
//     arch("symbol", symbol);
//     arch("modifiers", modifiers);
//     arch("state", state);
// }

// inline void MouseDownEvent::visit(Archive &arch) {
//     ARCH_PAIR(button);
//     ARCH_PAIR(x);
//     ARCH_PAIR(y);
// }

// inline void MouseMoveEvent::visit(Archive &arch) {
//     ARCH_PAIR(button);
//     ARCH_PAIR(x);
//     ARCH_PAIR(y);
// }

// inline void NullEvent::visit(Archive &arch) {}

// inline void PasteEvent::visit(Archive &arch) {
//     ARCH_PAIR(text);
// }

// inline void ResizeEvent::visit(Archive &arch) {
//     ARCH_PAIR(width);
//     ARCH_PAIR(height);
// }

// inline void from_json(const nlohmann::json &j, Event &e) {

//    auto &je = j["event"];
//    switch (size_t{j["event_type"]}) {
//    case 0:
//        e = NullEvent{};
//        break;
//    case 1: {
//        auto pe = KeyEvent{};
//        from_json(je, pe);
//        e = pe;
//        break;
//    }
//    case 2: {
//        auto pe = MouseDownEvent{};
//        from_json(je, pe);
//        e = pe;
//        break;
//    }
//    case 3: {
//        auto pe = MouseMoveEvent{};
//        from_json(je, pe);
//        e = pe;
//        break;
//    }
//    case 4: {
//        auto pe = PasteEvent{};
//        from_json(je, pe);
//        e = pe;
//        break;
//    }
//    case 5: {
//        auto pe = ResizeEvent{};
//        from_json(je, pe);
//        e = pe;
//        break;
//    }
//    }
//}

// inline void load(Archive &arch, Event &e) {
//     size_t t = 0;
//     arch("event_type", t);
//     switch (t) {
//     case 0:
//         e = NullEvent{};
//         break;
//     case 1: {
//         auto e = KeyEvent{};
//         break;
//     }
//     case 2: {
//         auto pe = MouseDownEvent{};
//         break;
//     }
//     case 3: {
//         auto pe = MouseMoveEvent{};
//         break;
//     }
//     case 4: {
//         auto pe = PasteEvent{};
//         break;
//     }
//     case 5: {
//         auto pe = ResizeEvent{};
//         break;
//     }
//     }

//    arch.beginChild("event");
//    std::visit([&arch](auto &e) { e.visit(arch); }, e);
//    arch.endChild();
//}

// inline void save(Archive &arch, Event &e) {
//     auto t = e.index();

//    arch("event_type", t);

//    arch.beginChild("event");
//    std::visit([&arch](auto &e) { e.visit(arch); }, e);
//    arch.endChild();
//}
