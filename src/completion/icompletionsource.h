#pragma once

#include "meditfwd.h"
#include "text/fstring.h"
#include <string>

class ICompletionSource {
public:
    struct CompletionItem {
        std::string name;
        FString description;
    };

    using CompletionList = std::vector<CompletionItem>;

    virtual ~ICompletionSource() = default;

    virtual CompletionList list(IEnvironment &env) = 0;

    virtual bool shouldComplete(IEnvironment &env) = 0;
};
