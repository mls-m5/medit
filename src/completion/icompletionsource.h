#pragma once

#include "meditfwd.h"
#include "text/fstring.h"
#include <memory>
#include <string>

class ICompletionSource {
public:
    struct CompletionItem {
        std::string name;
        FString description;
    };

    using CompletionList = std::vector<CompletionItem>;

    virtual ~ICompletionSource() = default;

    virtual CompletionList list(std::shared_ptr<IEnvironment>) = 0;

    virtual bool shouldComplete(std::shared_ptr<IEnvironment>) = 0;
};
