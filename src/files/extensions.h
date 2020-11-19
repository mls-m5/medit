#pragma once

#include "files/filesystem.h"

inline bool isCppHeader(const filesystem::path &path) {
    return path.extension() == ".h" || path.extension() == ".hpp";
}

inline bool isCppSource(const filesystem::path &path) {
    return path.extension() == ".cpp";
}

inline bool isCSource(const filesystem::path &path) {
    return path.extension() == ".c";
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

inline bool isMake(const filesystem::path &path) {
    return path.filename() == "Makefile";
}

inline bool isMatmake(const filesystem::path &path) {
    return path.filename() == "Matmakefile";
}

inline bool isKnownExtension(const filesystem::path &path) {
    return isCppHeader(path) || isCppSource(path) || isCSource(path) ||
           isCpp(path) || isJson(path) || isMarkdown(path) || isMake(path) ||
           isMatmake(path);
}
