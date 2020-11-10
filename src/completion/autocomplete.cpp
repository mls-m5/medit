

#include "autocomplete.h"
#include "plugin/clang/clangcompletion.h"
#include "wordcompletions.h"

AutoComplete::AutoComplete() {
    _sources.emplace_back(std::make_unique<ClangCompletion>());
    _sources.emplace_back(std::make_unique<WordCompletions>());
}

AutoComplete::~AutoComplete() {}

AutoComplete::CompletionList AutoComplete::getMatching(std::string beginning) {
    CompletionList ret;

    for (auto &item : _items) {
        if (item.name.rfind(beginning, 0) != std::string::npos) {
            ret.push_back(item);
        }
        //        ret.push_back(item);
    }

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
