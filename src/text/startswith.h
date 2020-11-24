#pragma once

#include <string>
#include <string_view>

//! To be used until all platforms supports std::string::start_with
inline bool starts_with(const std::string &str, std::string_view f) {
    return (str.rfind(f, 0) == 0);
}
