#pragma once

#include <chrono>
#include <iostream>

inline void debugOutput(std::string_view name) {
    using namespace std::chrono;

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto t = high_resolution_clock::now() - startTime;
    std::cerr << name << " " << duration_cast<milliseconds>(t).count() << "\n";
}
