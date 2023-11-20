#pragma once

#include "logtype.h"
#include <functional>
#include <sstream>
#include <string_view>

namespace impl {

void logInternal(LogType, std::string text);

}

// TODO: Maybe log with formatted text?

template <typename... Args>
void log(LogType type, const Args &...args) {
    auto ss = std::ostringstream{};

    (ss << ... << args);

    impl::logInternal(type, std::move(ss).str());
}

template <typename... Args>
void logInfo(const Args &...args) {
    log(LogType::Info, args...);
}

template <typename... Args>
void logError(const Args &...args) {
    log(LogType::Error, args...);
}

template <typename... Args>
void logStatusMessage(const Args &...args) {
    log(LogType::StatusMessage, args...);
}

void subscribeToLog(std::function<void(LogType, std::string_view)> callback);
void unsubscribeToLog();
