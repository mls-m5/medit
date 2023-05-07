#pragma once

#include "completion/icompletionsource.h"
#include "icompletionsource.h"
#include <memory>
#include <string>

class AutoComplete {
    std::vector<std::unique_ptr<ICompletionSource>> _sources;

public:
    using CompletionList = ICompletionSource::CompletionList;

    AutoComplete();
    ~AutoComplete();

    CompletionList getMatching(std::string beginning);

    void populate(std::shared_ptr<IEnvironment>,
                  std::function<void()> callback);

private:
    CompletionList _items;
};
