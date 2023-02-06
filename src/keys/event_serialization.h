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
    case 1:
        e = KeyEvent{je};
        break;
    case 2:
        e = MouseDownEvent{je};
        break;
    case 3:
        e = MouseMoveEvent{je};
        break;
    case 4:
        //        e = PasteEvent{je};
        e = PasteEvent{je["text"]};
        break;
    }
}
