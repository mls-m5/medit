#include "deserializescreen.h"
#include "nlohmann/json.hpp"
#include <sstream>

DeserializeScreen::DeserializeScreen(std::shared_ptr<IScreen> screen)
    : _screen{screen} {}

void DeserializeScreen::subscribe(
    std::function<void(std::string_view)> callback) {
    _callback = callback;
}

void DeserializeScreen::close() {
    _screen.reset();
}

void DeserializeScreen::write(std::string_view data) {
    handle(nlohmann::json::parse(data));
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
        _screen->draw(json["x"], json["y"], std::string{json["text"]});

        return;
    }

    if (method == "get/width") {
        send(nlohmann::json{
            {"id", id},
            {"value", _screen->width()},
        });
        return;
    }

    if (method == "get/height") {
        send(nlohmann::json{
            {"id", id},
            {"value", _screen->height()},
        });
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
}

void DeserializeScreen::send(const nlohmann::json &data) {
    throw std::runtime_error{"not implemented"};
}
