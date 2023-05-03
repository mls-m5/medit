#pragma once

#include "script/ienvironment.h"
#include "text/fstring.h"
#include "text/position.h"
#include <functional>
#include <memory>
#include <string>

class ICompletionSource {
public:
    struct TextEdit {
        /// Position in medit standard (0 is first)
        struct Range {
            Position begin;
            Position end;
        } range;

        std::string text;
    };

    struct CompletionItem {
        std::string name;
        FString description;
        std::string filterText;
        TextEdit edit;
    };

    using CompletionList = std::vector<CompletionItem>;

    using CompleteCallbackT = std::function<void(CompletionList)>;

    virtual ~ICompletionSource() = default;

    virtual void list(std::shared_ptr<IEnvironment>, CompleteCallbackT) = 0;

    virtual bool shouldComplete(std::shared_ptr<IEnvironment>) = 0;

    //! High priority means that it should be selected first
    virtual int priority() = 0;
};
