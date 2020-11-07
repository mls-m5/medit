#pragma once

#include "fstring.h"
#include <string>

class AutoComplete {
public:
    struct CompletionItem {
        std::string name;
        FString description;
    };
    using CompletionList = std::vector<CompletionItem>;

    AutoComplete();
    ~AutoComplete();

    CompletionList getMatching(std::string beginning);

    void populate(CompletionList items);

private:
    CompletionList _items;
};
