

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
        //        if (item.name.rfind(beginning, 0) == 0) {
        //            ret.push_back(item);
        //        }

        // Match parts (this does not work that good)
        if (item.name.find(beginning) != std::string::npos) {
            ret.push_back(item);
        }

        //         Just pass everything
        //        ret.push_back(item);
    }

    //    if (true) {
    //        if (ret.empty()) {
    //            ret = _items;
    //        }
    //    }

    return ret;
}

void AutoComplete::populate(std::shared_ptr<IEnvironment> env) {
    for (auto &source : _sources) {
        if (source->shouldComplete(env)) {
            _items = source->list(env);
            break;
        }
    }
}
