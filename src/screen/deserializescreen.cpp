#include "deserializescreen.h"
#include "keys/event_serialization.h"
#include "nlohmann/json.hpp"
#include "syntax/palette.h"
#include "text/fstring_serialization.h"
#include <sstream>
#include <thread>

DeserializeScreen::DeserializeScreen(std::shared_ptr<IScreen> screen)
    : _screen{screen} {
    _screen->subscribe(
        [this](IScreen::EventListT list) { screenCallback(std::move(list)); });
}

void DeserializeScreen::subscribe(
    std::function<void(std::string_view)> callback) {
    _callback = callback;
}

void DeserializeScreen::close() {
    _screen->unsubscribe();
    _screen.reset();
}

void DeserializeScreen::write(std::string_view data) {
    handle(nlohmann::json::parse(data));
}

void DeserializeScreen::unsubscribe() {
    _callback = {};
}

void DeserializeScreen::handle(const nlohmann::json &json) {
    long id = 0;
    if (auto f = json.find("id"); f != json.end()) {
        id = *f;
    }

    auto method = std::string{};
    if (auto f = json.find("method"); f != json.end()) {
        method = *f;
    }

    if (method == "draw") {
        _screen->draw(json["x"], json["y"], FString{json["text"]});
        return;
    }

    if (method == "cursor") {
        _screen->cursor(json["x"], json["y"]);
        return;
    }

    if (method == "cursorStyle") {
        _screen->cursorStyle(json["value"]);
        return;
    }

    if (method == "title") {
        _screen->title(json["value"]);
        return;
    }

    if (method == "clear") {
        _screen->clear();
        return;
    }

    if (method == "clear") {
        _screen->clear();
        return;
    }

    if (method == "refresh") {
        _screen->refresh();
        return;
    }

    if (method == "palette") {
        _screen->palette(json["value"]);
        return;
    }

    if (method == "set/clipboard") {
        _screen->clipboardData(json["value"]);
    }

    if (method == "get/clipboard") {
        send(nlohmann::json{
            {"id", id},
            {"value", _screen->clipboardData()},
        });
        return;
    }
}

void DeserializeScreen::send(const nlohmann::json &data) {
    auto ss = std::stringstream{};
    ss << data;
    if (!_callback) {
        _unhandledQueue.push_back(ss.str());
    }
    else {
        for (std::string_view item : _unhandledQueue) {
            _callback(item);
        }
        _unhandledQueue.clear();
        _callback(ss.str());
    }
}

void DeserializeScreen::screenCallback(IScreen::EventListT list) {
    send(nlohmann::json{{"events", list}});
}
