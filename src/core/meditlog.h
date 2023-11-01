#pragma once

#include <functional>
#include <sstream>
#include <string_view>

namespace impl {

void logInternal(std::string text);

}

// TODO: Maybe log with formatted text?

template <typename... Args>
void printInfo(Args... args) {
    auto ss = std::ostringstream{};

    (ss << ... << args);

    impl::logInternal(std::move(ss).str());
}

void subscribeToLog(std::function<void(std::string_view)> callback);
void unsubscribeToLog();
