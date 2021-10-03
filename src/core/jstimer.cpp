#ifdef __EMSCRIPTEN__
#include "jstimer.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <map>
#include <memory>

namespace {

struct CallbackType {
    std::function<void()> f;
    long i;
};

// Prevent momory leaks from when cancecling callbacks
std::map<long, std::unique_ptr<CallbackType>> callbackMap;

EMSCRIPTEN_KEEPALIVE
void call(void *ptr) {
    auto callback = static_cast<CallbackType *>(ptr);
    callback->f();
    callbackMap.erase(callback->i);
}

} // namespace

size_t JsTimer::setTimeout(DurationT duration, std::function<void()> f) {
    auto callback = std::make_unique<CallbackType>();
    callback->f = f;

    auto i = callback->i = emscripten_set_timeout(
        call,
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count(),
        callback.get());

    callbackMap[callback->i] = std::move(callback);

    return i;
}

void JsTimer::cancel(size_t ref) {
    emscripten_clear_timeout(ref);

    callbackMap.erase(ref);
}

void JsTimer::stop() {
    for (auto &pair : callbackMap) {
        emscripten_clear_timeout(pair.second->i);
    }
    callbackMap.clear();
}

void JsTimer::loop() {
    // Not implemented: Handled by js
}

void JsTimer::start() {
    // Not implemented: Handled by js
}

#endif // __EMSCRIPTEN__
