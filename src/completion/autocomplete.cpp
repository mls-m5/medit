

#include "completion/autocomplete.h"
#include "core/context.h"
#include "core/ijobqueue.h"
#include "core/plugins.h"
#include "script/ienvironment.h"
#include <algorithm>

AutoComplete::AutoComplete(Plugins::ListT<ICompletionSource> sources) {
    _sources = std::move(sources);

    std::sort(_sources.begin(), _sources.end(), [](auto &&a, auto &&b) {
        return a->priority() > b->priority();
    });
}

AutoComplete::~AutoComplete() {}

AutoComplete::CompletionList AutoComplete::getMatching(std::string beginning) {
    CompletionList ret;

    for (auto &item : _items) {
        if (item.filterText.find(beginning) != std::string::npos) {
            ret.push_back(item);
        }
    }

    return ret;
}

void AutoComplete::populate(std::shared_ptr<IEnvironment> env,
                            std::function<void()> callback) {
    _items.clear();

    for (auto &source : _sources) {
        if (source->shouldComplete(env)) {
            auto cb =
                [this, env, callback](ICompletionSource::CompletionList list) {
                    env->context().guiQueue().addTask([this, list, callback] {
                        this->_items = list;
                        callback();
                    });
                };
            source->list(env, cb);
            break;
        }
    }
}
