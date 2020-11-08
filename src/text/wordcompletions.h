#pragma once

#include "icompletionsource.h"

class WordCompletions : public ICompletionSource {
public:
    // @see ICompletions
    CompletionList list(IEnvironment &) override {
        using namespace std::literals;
        return {
            {"apa", {"a class"s, 2}},
            {"apa2", {"a class"s, 2}},
            {"bepa", {"a function"s, 2}},
            {"cepa", {"a function"s, 2}},
        };
    }
};
