#include "meditlog.h"
#include "core/logtype.h"
#include <iostream>
#include <mutex>
#include <queue>
#include <streambuf>
#include <string>

namespace {

class CustomStreamBuf : public std::streambuf {
private:
    std::string currentString;

protected:
    int_type overflow(int_type ch) override {
        if (ch != traits_type::eof()) {
            currentString += static_cast<char>(ch);
            if (ch == '\n') {
                impl::logInternal(LogType::Info, currentString);
                currentString.clear();
            }
        }
        return ch;
    }

    int sync() override {
        if (!currentString.empty()) {
            impl::logInternal(LogType::Info, currentString);
            currentString.clear();
        }
        return 0;
    }

public:
    CustomStreamBuf() {}
    ~CustomStreamBuf() override {
        // Ensure everything is flushed
        CustomStreamBuf::sync();
    }
};

class MeditLog {
public:
    ~MeditLog() {
        // If the program is closed without the text being printed
        while (!loggedText.empty()) {
            auto lock = std::unique_lock{_mutex};

            std::cout << loggedText.front().second << "\n";
            loggedText.pop();
        }
    }

    void log(LogType type, std::string text) {
        auto lock = std::unique_lock{_mutex};

        if (!_callback) {
            loggedText.push({type, std::move(text)});
            return;
        }

        _callback(type, text);
    }

    static MeditLog &instance() {
        static MeditLog log;
        return log;
    }

    std::queue<std::pair<LogType, std::string>> loggedText;

    void subscribe(std::function<void(LogType, std::string_view)> callback) {
        _callback = callback;

        // Flush the cached text
        while (!loggedText.empty()) {
            auto lock = std::unique_lock{_mutex};

            _callback(loggedText.front().first, loggedText.front().second);
            loggedText.pop();
        }
    }

    void unsubscribe() {
        _callback = {};
    }

    std::function<void(LogType, std::string_view)> _callback;
    std::mutex _mutex;

    CustomStreamBuf buf;
    std::streambuf *originalBuf = nullptr;
};

} // namespace

void impl::logInternal(LogType type, std::string text) {
    if (type == LogType::StatusMessage) {
        for (auto &c : text) {
            if (c == '\n') {
                c = ' ';
            }
        }
    }
    MeditLog::instance().log(type, std::move(text));
}

void subscribeToLog(std::function<void(LogType, std::string_view)> callback) {
    auto &instance = MeditLog::instance();
    instance.originalBuf = std::cout.rdbuf();
    std::cout.rdbuf(&instance.buf);
    instance.subscribe(callback);
}

void unsubscribeToLog() {
    auto &instance = MeditLog::instance();
    instance.unsubscribe();
    if (instance.originalBuf) {
        std::cout.rdbuf(instance.originalBuf);
    }
}
