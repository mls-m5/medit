#pragma once

#include "meditfwd.h"
#include "text/fstring.h"
#include <functional>
#include <memory>
#include <string>

class ICompletionSource {
public:
    struct CompletionItem {
        std::string name;
        FString description;
    };

    using CompletionList = std::vector<CompletionItem>;

    using CompleteCallbackT = std::function<void(CompletionList)>;

    virtual ~ICompletionSource() = default;

    virtual void list(std::shared_ptr<IScope>, CompleteCallbackT) = 0;

    virtual bool shouldComplete(std::shared_ptr<IScope>) = 0;

    //! High priority means that it should be selected first
    virtual int priority() = 0;
};
