

#include "completion/autocomplete.h"
#include "core/plugins.h"
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

void AutoComplete::populate(std::shared_ptr<IScope> env) {
    for (auto &source : _sources) {
        if (source->shouldComplete(env)) {
            source->list(env, [this](ICompletionSource::CompletionList list) {
                this->_items = list;
            });
            break;
        }
    }
}
