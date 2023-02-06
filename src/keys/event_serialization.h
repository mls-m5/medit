#pragma once

#include "keys/event.h"
#include "nlohmann/json.hpp"
#include "text/utf8char_serialization.h"

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(KeyEvent, key, symbol, modifiers, state);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MouseDownEvent, button, x, y);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MouseMoveEvent, button, x, y);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PasteEvent, text);

inline void to_json(nlohmann::json &j, const Event &e) {
    j["event_type"] = e.index();
    std::visit([&j](auto &&e) { j["event"] = e; }, e);
}

inline void from_json(const nlohmann::json &j, Event &e) {

    auto &je = j["event"];
    switch (size_t{j["event_type"]}) {
    case 0:
        e = NullEvent{};
        break;
    case 1: {
        auto pe = KeyEvent{};
        from_json(je, pe);
        e = pe;
        break;
    }
    case 2: {
        auto pe = MouseDownEvent{};
        from_json(je, pe);
        e = pe;
        break;
    }
    case 3: {
        auto pe = MouseMoveEvent{};
        from_json(je, pe);
        e = pe;
        break;
    }
    case 4: {
        auto pe = PasteEvent{};
        from_json(je, pe);
        e = pe;
        break;
    }
    }
}
