#pragma once

#include <random>

namespace lsp {

inline auto &randGen() {
    static auto gen = std::mt19937{std::random_device{}()};
    return gen;
}

template <typename T>
inline auto randomNumber(T max) {
    return std::uniform_int_distribution<T>(0, max)(randGen());
}

} // namespace lsp
