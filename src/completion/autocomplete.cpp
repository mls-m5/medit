

#include "completion/autocomplete.h"
#include "completion/wordcompletions.h"
#include "clang/clangcompletion.h"

AutoComplete::AutoComplete() {
    _sources.emplace_back(std::make_unique<ClangCompletion>());
    _sources.emplace_back(std::make_unique<WordCompletions>());
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

void AutoComplete::populate(IEnvironment &env) {
    for (auto &source : _sources) {
        if (source->shouldComplete(env)) {
            _items = source->list(env);
            break;
        }
    }
}
