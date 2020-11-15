#pragma once

#include "files/filesystem.h"

inline bool isCppHeader(const filesystem::path &path) {
    return path.extension() == ".h";
}

inline bool isCppSource(const filesystem::path &path) {
    return path.extension() == ".cpp";
}

inline bool isCpp(const filesystem::path &path) {
    return isCppHeader(path) || isCppSource(path);
}

inline bool isJson(const filesystem::path &path) {
    return path.extension() == ".json";
}

inline bool isMarkdown(const filesystem::path &path) {
    return path.extension() == ".md";
}
