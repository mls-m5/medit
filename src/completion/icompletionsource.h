#pragma once

#include "meditfwd.h"
#include "text/cursorrange.h"
#include "text/fstring.h"
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
        TextEdit edit;
    };

    using CompletionList = std::vector<CompletionItem>;

    using CompleteCallbackT = std::function<void(CompletionList)>;

    virtual ~ICompletionSource() = default;

    virtual void list(std::shared_ptr<IScope>, CompleteCallbackT) = 0;

    virtual bool shouldComplete(std::shared_ptr<IScope>) = 0;

    //! High priority means that it should be selected first
    virtual int priority() = 0;
};
