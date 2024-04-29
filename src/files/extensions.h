#pragma once

#include <array>
#include <filesystem>

inline bool isCppHeader(const std::filesystem::path &path) {
    auto ext = path.extension();
    return ext == ".h" || ext == ".hpp" || ext == ".hxx";
}

inline bool isCppSource(const std::filesystem::path &path) {
    auto ext = path.extension();
    return ext == ".cpp" || ext == ".cc" || ext == ".cxx" || ext == "cppm";
}

inline bool isCSource(const std::filesystem::path &path) {
    return path.extension() == ".c";
}

inline constexpr auto isC = isCSource;

inline bool isCpp(const std::filesystem::path &path) {
    return isCppHeader(path) || isCppSource(path);
}

inline bool isJson(const std::filesystem::path &path) {
    return path.extension() == ".json";
}

inline bool isMarkdown(const std::filesystem::path &path) {
    return path.extension() == ".md";
}

inline bool isMake(const std::filesystem::path &path) {
    return path.filename() == "Makefile";
}

inline bool isMatmake(const std::filesystem::path &path) {
    return path.filename() == "Matmakefile";
}

inline bool isCmakeLists(const std::filesystem::path &path) {
    return path.filename() == "CMakeLists.txt";
}

inline bool isTextFile(const std::filesystem::path &path) {
    return path.extension() == ".txt";
}

inline bool isHtml(const std::filesystem::path &path) {
    return path.extension() == ".html";
}

inline bool isXhtml(const std::filesystem::path &path) {
    return path.extension() == ".html";
}

inline bool isXml(const std::filesystem::path &path) {
    return path.extension() == ".xml";
}

inline bool isSvg(const std::filesystem::path &path) {
    return path.extension() == ".svg";
}

inline bool isJs(const std::filesystem::path &path) {
    return path.extension() == ".js";
}

inline bool isPython(const std::filesystem::path &path) {
    return path.extension() == ".py";
}

inline bool isJava(const std::filesystem::path &path) {
    return path.extension() == ".java";
}

inline bool isCSharp(const std::filesystem::path &path) {
    return path.extension() == ".cs";
}

inline bool isPhp(const std::filesystem::path &path) {
    return path.extension() == ".php";
}

inline bool isGo(const std::filesystem::path &path) {
    return path.extension() == ".go";
}

inline bool isRust(const std::filesystem::path &path) {
    return path.extension() == ".rs";
}

inline bool isGitIgnore(const std::filesystem::path &path) {
    return path.has_filename() && path.filename() == ".gitignore";
}

inline bool isClangFormat(const std::filesystem::path &path) {
    return path.has_filename() && path.filename() == ".clang-format";
}

inline bool isMsk(const std::filesystem::path &path) {
    return path.extension() == ".msk" || path.extension() == ".msp";
}

inline bool isKnownExtension(const std::filesystem::path &path) {
    const auto checks = std::to_array(
        {isCppHeader, isCppSource, isCSource,     isCpp,        isJson,
         isMarkdown,  isMake,      isMatmake,     isCmakeLists, isTextFile,
         isHtml,      isJs,        isPython,      isJava,       isCSharp,
         isPhp,       isGo,        isXml,         isSvg,        isXhtml,
         isRust,      isGitIgnore, isClangFormat, isMsk});

    for (const auto &check : checks) {
        if (check(path)) {
            return true;
        }
    }

    return false;
}
