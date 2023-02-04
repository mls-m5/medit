#include "serializescreen.h"
#include "json/json.h"

SerializeScreen::SerializeScreen(std::shared_ptr<IConnection> connection)
    : _connection{connection} {
    connection->subscribe([this](std::string_view str) {
        auto ss = std::istringstream{std::string{str}};
        auto json = Json{};
        ss >> json;
    });
}

SerializeScreen::~SerializeScreen() {
    _connection->close();
}

void SerializeScreen::draw(size_t x, size_t y, const FString &str) {
    //    Json json;
    //    json["method"] = "draw";
    //    json["x"] = x;
    //    json["y"] = y;
    //    json["text"] = std::string{str};
    //    send(json);
}

void SerializeScreen::refresh() {
    Json json;
    json["method"] = "refresh";
    send(json);
}

void SerializeScreen::clear() {
    Json json;
    json["method"] = "clear";
    send(json);
}

void SerializeScreen::cursor(size_t x, size_t y) {
    Json json;
    json["method"] = "clear";
    json["x"] = x;
    json["y"] = y;
    send(json);
}

size_t SerializeScreen::x() const {
    Json json;
    json["method"] = "get/x";
    auto res = const_cast<SerializeScreen &>(*this).request(json);
    return res;
}

size_t SerializeScreen::y() const {
    Json json;
    json["method"] = "get/x";
    auto res = const_cast<SerializeScreen &>(*this).request(json);
    return res;
}

size_t SerializeScreen::width() const {
    Json json;
    json["method"] = "get/width";
    auto res = const_cast<SerializeScreen &>(*this).request(json);
    return res;
}

size_t SerializeScreen::height() const {
    Json json;
    json["method"] = "get/height";
    auto res = const_cast<SerializeScreen &>(*this).request(json);
    return res;
}

void SerializeScreen::title(std::string title) {
    Json json;
    json["method"] = "title";
    json["title"] = title;
    send(json);
}

const IPalette &SerializeScreen::palette() const {}

IPalette &SerializeScreen::palette() {}

size_t SerializeScreen::addStyle(const Color &foreground,
                                 const Color &background,
                                 size_t index) {
    Json json;
    json["method"] = "addStyle";
    //    json["title"] = title;
    send(json);
    // TODO: Implement this
    return 0;
}

void SerializeScreen::cursorStyle(CursorStyle style) {
    Json json;
    json["method"] = "cursorStyle";
    json["style"] = static_cast<int>(style);
    send(json);
}

void SerializeScreen::send(const Json &json) {
    auto ss = std::stringstream{};
    ss << json;
    _connection->write(ss.str());
}

Json SerializeScreen::request(Json json) {
    auto id = ++_currentRequest;
    json["id"] = id;
    auto ss = std::stringstream{};
    ss << json;
    _connection->write(ss.str());

    auto lock = std::unique_lock(_mutex);
    _cv.wait(lock, [this, id] { return _receivedRequest = id; });

    lock.lock();
    return std::move(_receivedData);
}

void SerializeScreen::receive(Json json) {
    if (auto it = json.find("id"); it != json.end()) {
        auto lock = std::unique_lock(_mutex);
        _receivedRequest = std::stol(it->value);
        _receivedData = std::move(json); // Note: json + it is moved from
    }
    else {
        return;
    }
    _cv.notify_all();
}
