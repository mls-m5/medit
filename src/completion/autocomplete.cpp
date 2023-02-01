

#include "completion/autocomplete.h"
#include "core/context.h"
#include "core/jobqueue.h"
#include "core/plugins.h"
#include "script/ienvironment.h"
#include "script/iscope.h"
#include <algorithm>

AutoComplete::AutoComplete() {
    _sources = createCompletionSources();

    std::sort(_sources.begin(), _sources.end(), [](auto &&a, auto &&b) {
        return a->priority() > b->priority();
    });
}

AutoComplete::~AutoComplete() {}

AutoComplete::CompletionList AutoComplete::getMatching(std::string beginning) {
    CompletionList ret;

    for (auto &item : _items) {
        // Match parts (this does not work that good)
        if (item.name.find(beginning) != std::string::npos) {
            ret.push_back(item);
        }
    }

    return ret;
}

void AutoComplete::populate(std::shared_ptr<IScope> scope) {
    for (auto &source : _sources) {
        if (source->shouldComplete(scope)) {
            auto cb = [this, scope](ICompletionSource::CompletionList list) {
                scope->env().context().guiQueue().addTask(
                    [this, list] { this->_items = list; });
            };
            source->list(scope, cb);
            break;
        }
    }
}
