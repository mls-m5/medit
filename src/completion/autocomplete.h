#pragma once

#include "completion/icompletionsource.h"
#include "core/plugins.h"
#include "icompletionsource.h"
#include "meditfwd.h"
#include <memory>
#include <string>

class AutoComplete {
    std::vector<std::shared_ptr<ICompletionSource>> _sources;

public:
    using CompletionList = ICompletionSource::CompletionList;

    AutoComplete(Plugins::ListT<ICompletionSource>);
    ~AutoComplete();

    CompletionList getMatching(std::string beginning);

    void populate(std::shared_ptr<IEnvironment>,
                  std::function<void()> callback);

private:
    CompletionList _items;
};
