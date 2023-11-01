#include "meditlog.h"
#include <mutex>
#include <queue>
#include <string>

namespace {

class MeditLog {
public:
    void log(std::string text) {
        auto lock = std::unique_lock{_mutex};

        if (!_callback) {
            loggedText.push(std::move(text));
            return;
        }

        _callback(text);
    }

    static MeditLog &instance() {
        static MeditLog log;
        return log;
    }

    std::queue<std::string> loggedText;

    void subscribe(std::function<void(std::string_view)> callback) {
        _callback = callback;

        // Flush the cached text
        while (!loggedText.empty()) {
            auto lock = std::unique_lock{_mutex};

            _callback(loggedText.front());
            loggedText.pop();
        }
    }

    void unsubscribe() {
        _callback = {};
    }

    std::function<void(std::string_view)> _callback;
    std::mutex _mutex;
};

} // namespace

void impl::logInternal(std::string text) {
    MeditLog::instance().log(std::move(text));
}

void subscribeToLog(std::function<void(std::string_view)> callback) {
    MeditLog::instance().subscribe(callback);
}

void unsubscribeToLog() {
    MeditLog::instance().unsubscribe();
}
