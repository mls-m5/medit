#include "serializescreen.h"
#include "core/inarchive.h"
#include "core/outarchive.h"
#include "core/profiler.h"
#include "syntax/palette.h"
#include <sstream>
#include <string_view>

using namespace std::literals;

SerializeScreen::SerializeScreen(std::shared_ptr<IConnection> connection)
    : _connection{connection} {
    connection->subscribe([this](std::string_view str) { receive(str); });
}

SerializeScreen::~SerializeScreen() {
    _connection->close();
}

void SerializeScreen::draw(size_t x, size_t y, FStringView str) {
    PROFILE_FUNCTION();
    auto ss = std::ostringstream{};
    {
        auto arch = OutArchive{ss};
        arch.set("method", "draw"s);
        arch.set("x", static_cast<double>(x));
        arch.set("y", static_cast<double>(y));
        arch.set("text", str);
    }
    send(std::string_view{ss.str()});
}

void SerializeScreen::refresh() {
    PROFILE_FUNCTION();
    auto ss = std::ostringstream{};
    {
        auto arch = OutArchive{ss};
        arch.set("method", "refresh"s);
    }
    send(std::string_view{ss.str()});
}

void SerializeScreen::clear() {
    PROFILE_FUNCTION();
    auto ss = std::ostringstream{};
    {
        auto arch = OutArchive{ss};
        arch.set("method", "clear"s);
    }
    send(std::string_view{ss.str()});
}

void SerializeScreen::cursor(size_t x, size_t y) {
    PROFILE_FUNCTION();
    auto ss = std::ostringstream{};
    {
        auto arch = OutArchive{ss};
        arch.set("method", "cursor"s);
        arch.set("x", static_cast<double>(x));
        arch.set("y", static_cast<double>(y));
    }
    send(std::string_view{ss.str()});
}

void SerializeScreen::title(std::string title) {
    PROFILE_FUNCTION();
    auto ss = std::ostringstream{};
    {
        auto arch = OutArchive{ss};
        arch.set("method", "title"s);
        arch("value", title);
    }
    send(std::string_view{ss.str()});
}

void SerializeScreen::palette(const Palette &palette) {
    PROFILE_FUNCTION();
    auto ss = std::ostringstream{};
    {
        auto arch = OutArchive{ss};
        arch.set("method", "palette"s);
        arch.set("value", palette);
    }
    send(std::string_view{ss.str()});
}

size_t SerializeScreen::addStyle(const Color &foreground,
                                 const Color &background,
                                 size_t index) {
    PROFILE_FUNCTION();
    auto ss = std::ostringstream{};
    {
        auto arch = OutArchive{ss};
        arch.set("method", "add_style"s);
        auto c = foreground;
        arch("foreground", c);
        c = background;
        arch("background", c);
        arch("index", index);
    }
    send(std::string_view{ss.str()});
    // TODO: Implement this
    return 0;
}

void SerializeScreen::cursorStyle(CursorStyle style) {
    PROFILE_FUNCTION();
    auto ss = std::ostringstream{};
    {
        auto arch = OutArchive{ss};
        arch.set("method", "cursorStyle"s);
        arch("value", style);
    }
    send(std::string_view{ss.str()});
}

void SerializeScreen::subscribe(CallbackT f) {
    _callback = f;
}

void SerializeScreen::unsubscribe() {
    _callback = {};
}

std::string SerializeScreen::clipboardData() {
    PROFILE_FUNCTION();
    return this->request("get/clipboard");
}

void SerializeScreen::clipboardData(std::string text) {
    PROFILE_FUNCTION();
    auto ss = std::ostringstream{};
    {
        auto arch = OutArchive{ss};
        arch.set("method", "set/clipboard"s);
        arch("value", text);
    }
    send(std::string_view{ss.str()});
}

void SerializeScreen::send(std::string_view str) {
    PROFILE_FUNCTION();
    _connection->write(str);
}

std::string SerializeScreen::request(std::string_view method) {
    PROFILE_FUNCTION();
    auto id = ++_currentRequest;
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

void SerializeScreen::receive(std::string_view str) {
    PROFILE_FUNCTION();
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
