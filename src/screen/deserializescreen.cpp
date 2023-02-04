#include "deserializescreen.h"
#include "json/json.h"
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
    auto ss = std::ostringstream{std::string{data}};

    auto json = Json::Parse(std::string{data});

    handle(json);
}

void DeserializeScreen::handle(const Json &json) {
    long id = 0;
    if (auto f = json.find("id"); f != json.end()) {
        id = std::stol(f->value);
    }

    auto method = std::string{};
    if (auto f = json.find("method"); f != json.end()) {
        method = f->string();
    }

    if (method == "width") {
        auto r = Json{};
        r["id"] = id;
        r["value"] = _screen->width();
        send(r);
        return;
    }

    if (method == "height") {
        auto r = Json{};
        r["id"] = id;
        r["value"] = _screen->height();
        send(r);
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

void DeserializeScreen::send(const Json &data) {
    throw std::runtime_error{"not implemented"};
}
