#pragma once

#include "files/filesystem.h"

inline bool isCppHeader(const filesystem::path &path) {
    auto ext = path.extension();
    return ext == ".h" || ext == ".hpp" || ext == ".hxx";
}

inline bool isCppSource(const filesystem::path &path) {
    auto ext = path.extension();
    return ext == ".cpp" || ext == ".cc" || ext == ".cxx" || ext == "cppm";
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

inline bool isCmakeLists(const filesystem::path &path) {
    return path.filename() == "CMakeLists.txt";
}

inline bool isTextFile(const filesystem::path &path) {
    return path.extension() == ".txt";
}

inline bool isHtml(const filesystem::path &path) {
    return path.extension() == ".html";
}

inline bool isJs(const filesystem::path &path) {
    return path.extension() == ".js";
}

inline bool isPython(const filesystem::path &path) {
    return path.extension() == ".js";
}

inline bool isJava(const filesystem::path &path) {
    return path.extension() == ".java";
}

inline bool isCSharp(const filesystem::path &path) {
    return path.extension() == ".cs";
}

inline bool isPhp(const filesystem::path &path) {
    return path.extension() == ".php";
}

inline bool isKnownExtension(const filesystem::path &path) {
    return isCppHeader(path) || isCppSource(path) || isCSource(path) ||
           isCpp(path) || isJson(path) || isMarkdown(path) || isMake(path) ||
           isMatmake(path) || isCmakeLists(path) || isTextFile(path) ||
           isHtml(path) || isJs(path) || isPython(path) || isJava(path) ||
           isCSharp(path) || isPhp(path);
}
