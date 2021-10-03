#pragma once

#include "files/filesystem.h"
#include <stdexcept>
#include <string>

enum Os {
    Linux,
    Windows,
    Emscripten,
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define MATMAKE_USING_WINDOWS
#endif
constexpr Os getOs() {
#ifdef MEDIT_USING_WINDOWS
    return Os::Windows;
#else
#ifdef __EMSCRIPTEN__
    return Os::Emscripten;
#else
    return Os::Linux;
#endif // __EMSCRIPTEN
#endif
}

inline bool hasCommand(std::string command) {
    if constexpr (getOs() == Os::Linux) {
        return !system(("command -v " + command + " > /dev/null").c_str());
    }
    else {
        throw std::runtime_error{std::string{__FILE__} + ":" +
                                 std::to_string(__LINE__) +
                                 " is not implemented "};
    }
}

filesystem::path executablePath();
