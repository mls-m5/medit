#include "serializescreen.h"
#include "core/outarchive.h"
#include "keys/event_serialization.h"
// #include "nlohmann/json.hpp"
#include "core/inarchive.h"
#include "syntax/palette.h"
#include "text/fstring_serialization.h"
#include "text/fstringview_serialization.h"
#include <sstream>
#include <string_view>

SerializeScreen::SerializeScreen(std::shared_ptr<IConnection> connection)
    : _connection{connection} {
    connection->subscribe([this](std::string_view str) { receive(str); });
}

SerializeScreen::~SerializeScreen() {
    _connection->close();
}

void SerializeScreen::draw(size_t x, size_t y, FStringView str) {
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
    return const_cast<SerializeScreen *>(this)->request("get/clipboard");
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

nlohmann::json SerializeScreen::request(std::string_view method) {
    int id = ++_currentRequest;
    {
        auto ss = std::ostringstream{};
        {
            OutArchive oarch{ss};
            oarch("id", id);
            auto methodstr = std::string{method};
            oarch("method", methodstr);
        };
        _connection->write(ss.str());
    }

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

// void SerializeScreen::receive(const nlohmann::json &json) {
// void SerializeScreen::receive(Archive &arch) {
void SerializeScreen::receive(std::string_view str) {
    auto ss = std::istringstream{std::string{str}};
    auto arch = InArchive{ss};

    long id = 0;

    if (arch("id", id)) {
        auto lock = std::unique_lock(_mutex);
        _receivedRequest = id;
        _receivedData = arch.get<std::string>("value");
    }
    else {
        std::vector<Event> e; //  = json;
        arch("events", e);
        _callback(e);

        return;
    }
    _cv.notify_all();
}
