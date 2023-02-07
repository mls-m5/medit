#include "serializescreen.h"
#include "keys/event_serialization.h"
#include "nlohmann/json.hpp"
#include "syntax/palette.h"
#include "text/fstring_serialization.h"
#include <sstream>

SerializeScreen::SerializeScreen(std::shared_ptr<IConnection> connection)
    : _connection{connection} {
    connection->subscribe([this](std::string_view str) {
        auto ss = std::istringstream{std::string{str}};
        auto json = nlohmann::json{};
        ss >> json;
        receive(json);
    });
}

SerializeScreen::~SerializeScreen() {
    _connection->close();
}

void SerializeScreen::draw(size_t x, size_t y, const FString &str) {
    send(nlohmann::json{
        {"method", "draw"},
        {"x", static_cast<double>(x)},
        {"y", static_cast<double>(y)},
        {"text", str},
    });
}

void SerializeScreen::refresh() {
    send(nlohmann::json{
        {"method", "refresh"},
    });
}

void SerializeScreen::clear() {
    send(nlohmann::json{
        {"method", "clear"},
    });
}

void SerializeScreen::cursor(size_t x, size_t y) {
    send(nlohmann::json{
        {"method", "cursor"},
        {"x", static_cast<double>(x)},
        {"y", static_cast<double>(y)},
    });
}

// size_t SerializeScreen::x() const {
//     auto r = const_cast<SerializeScreen *>(this)->request({
//         {"method", "get/x"},
//     });
//     return r["value"];
// }

// size_t SerializeScreen::y() const {
//     auto r = const_cast<SerializeScreen *>(this)->request({
//         {"method", "get/y"},
//     });
//     return r["value"];
// }

// size_t SerializeScreen::width() const {
//     auto r = const_cast<SerializeScreen *>(this)->request({
//         {"method", "get/width"},
//     });
//     return r["value"];
// }

// size_t SerializeScreen::height() const {
//     auto r = const_cast<SerializeScreen *>(this)->request({
//         {"method", "get/height"},
//     });
//     return r["value"];
//}

void SerializeScreen::title(std::string title) {
    send(nlohmann::json{
        {"method", "title"},
        {"value", title},
    });
}

// const IPalette &SerializeScreen::palette() const {}

void SerializeScreen::palette(const Palette &palette) {
    send({
        {"method", "palette"},
        {"value", palette},
    });
}

size_t SerializeScreen::addStyle(const Color &foreground,
                                 const Color &background,
                                 size_t index) {
    //    Json json;
    //    json["method"] = "addStyle";
    //    json["title"] = title;
    //    send(json);
    // TODO: Implement this
    return 0;
}

void SerializeScreen::cursorStyle(CursorStyle style) {
    send({
        {"method", "cursorStyle"},
        {"value", static_cast<double>(style)},
    });
}

void SerializeScreen::subscribe(CallbackT f) {
    _callback = f;
}

void SerializeScreen::unsubscribe() {
    _callback = {};
}

std::string SerializeScreen::clipboardData() {
    auto r = const_cast<SerializeScreen *>(this)->request({
        {"method", "get/clipboard"},
    });
    return r["value"];
}

void SerializeScreen::clipboardData(std::string text) {
    send({
        {"method", "set/clipboard"},
        {"value", text},
    });
}

void SerializeScreen::send(const nlohmann::json &json) {
    auto ss = std::stringstream{};
    ss << json;
    _connection->write(ss.str());
}

nlohmann::json SerializeScreen::request(nlohmann::json json) {
    int id = ++_currentRequest;
    json["id"] = static_cast<double>(id);
    auto ss = std::stringstream{};
    ss << json;
    _connection->write(ss.str());

    {
        auto lock = std::unique_lock(_mutex);

        // The response arrived before started looking for it
        if (_receivedRequest == id) {
            return std::move(_receivedData);
        }

        _cv.wait(lock, [this, id] { return _receivedRequest == id; });
    }

    {
        auto lock = std::unique_lock(_mutex);
        return std::move(_receivedData);
    }
}

void SerializeScreen::receive(const nlohmann::json &json) {
    if (auto it = json.find("id"); it != json.end()) {
        auto lock = std::unique_lock(_mutex);
        _receivedRequest = it->get<long>();
        _receivedData = std::move(json); // Note: json + it is moved from
    }
    else {
        std::vector<Event> e = json;
        _callback(e);

        return;
    }
    _cv.notify_all();
}
