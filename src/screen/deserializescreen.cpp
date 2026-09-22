#include "deserializescreen.h"
#include "core/inarchive.h"
#include "core/outarchive.h"
#include "core/profiler.h"
#include "screen/cursorstyle.h"
#include "syntax/palette.h"
#include <sstream>
#include <string_view>
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
    PROFILE_FUNCTION();
    handle(data);
}

void DeserializeScreen::unsubscribe() {
    PROFILE_FUNCTION();
    _callback = {};
}

void DeserializeScreen::handle(std::string_view str) {
    PROFILE_FUNCTION();
    auto ss = std::istringstream{std::string{str}};
    auto arch = InArchive{ss};

    long id = 0;

    arch("id", id);

    auto method = arch.get<std::string>("method");

    if (method == "draw") {
        _screen->draw(arch.get<size_t>("x"),
                      arch.get<size_t>("y"),
                      arch.get<FString>("text"));
        return;
    }

    if (method == "cursor") {
        _screen->cursor(arch.get<size_t>("x"), arch.get<size_t>("y"));
        return;
    }

    if (method == "cursorStyle") {
        _screen->cursorStyle(arch.get<CursorStyle>("value"));
        return;
    }

    if (method == "title") {
        _screen->title(arch.get<std::string>("value"));
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
        _screen->palette(arch.get<Palette>("value"));
        return;
    }

    if (method == "set/clipboard") {
        _screen->clipboardData(arch.get<std::string>("value"));
    }

    if (method == "get/clipboard") {
        auto oss = std::ostringstream{};
        {
            auto oarch = OutArchive{oss};

            oarch.set("id", id);
            oarch.set("value", _screen->clipboardData());
        }
        send(oss.str());
        return;
    }
}

void DeserializeScreen::send(std::string_view str) {
    PROFILE_FUNCTION();
    //    auto ss = std::stringstream{};
    //    ss << data;
    if (!_callback) {
        _unhandledQueue.push_back(std::string{str});
    }
    else {
        for (std::string_view item : _unhandledQueue) {
            _callback(item);
        }
        _unhandledQueue.clear();
        _callback(str);
    }
}

void DeserializeScreen::screenCallback(IScreen::EventListT list) {
    PROFILE_FUNCTION();

    auto ss = std::ostringstream{};
    {
        auto oarch = OutArchive{ss};
        oarch("events", list);
    }
    send(ss.str());
}
