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
void logInfo(Args... args) {
    auto ss = std::ostringstream{};

    (ss << ... << args);

    impl::logInternal(LogType::ConsoleInfo, std::move(ss).str());
}

template <typename... Args>
void logError(Args... args) {
    auto ss = std::ostringstream{};

    (ss << ... << args);

    impl::logInternal(LogType::Error, std::move(ss).str());
}

template <typename... Args>
void logStatusMessage(Args... args) {
    auto ss = std::ostringstream{};

    (ss << ... << args);

    impl::logInternal(LogType::StatusMessage, std::move(ss).str());
}

void subscribeToLog(std::function<void(LogType, std::string_view)> callback);
void unsubscribeToLog();
