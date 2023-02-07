#pragma once

#include <chrono>
#include <iostream>

template <typename... Args>
inline void debugOutput(const Args &...args) {
    using namespace std::chrono;

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto t = high_resolution_clock::now() - startTime;
    std::cerr << duration_cast<milliseconds>(t).count() << "\t";
    ((std::cerr << " " << args), ...);
    std::cerr << "\n";
}
