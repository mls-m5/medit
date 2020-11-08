#pragma once

#include "fstring.h"
#include "icompletionsource.h"
#include "text/icompletionsource.h"
#include <memory>
#include <string>

class AutoComplete {

    std::unique_ptr<ICompletionSource> _source;

public:
    using CompletionList = ICompletionSource::CompletionList;

    AutoComplete();
    ~AutoComplete();

    CompletionList getMatching(std::string beginning);

    void populate(IEnvironment &);

private:
    CompletionList _items;
};
