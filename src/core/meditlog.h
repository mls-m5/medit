#pragma once

#include <sstream>
#include <string_view>

namespace impl {

void logInternal(std::string text);

}

// TODO: Maybe log with formatted text?

template <typename... Args>
void log(std::string_view sourceName, Args... args) {
    auto ss = std::ostringstream{};

    (ss << ... << args) << std::endl;

    impl::logInternal(std::move(ss).str());
}
