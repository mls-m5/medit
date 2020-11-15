#pragma once

#include "files/filesystem.h"

inline bool isCpp(const filesystem::path &path) {
    auto extension = path.extension();
    return extension == ".h" || extension == ".cpp";
}

inline bool isJson(const filesystem::path &path) {
    return path.extension() == ".json";
}

inline bool isMarkdown(const filesystem::path &path) {
    return path.extension() == ".md";
}
