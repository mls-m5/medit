

#include "autocomplete.h"
#include "wordcompletions.h"

AutoComplete::AutoComplete() {
    _source = std::make_unique<WordCompletions>();
}

AutoComplete::~AutoComplete() {}

AutoComplete::CompletionList AutoComplete::getMatching(std::string beginning) {
    CompletionList ret;

    for (auto &item : _items) {
        //        if (item.name.find(beginning) != std::string::npos) {
        if (item.name.starts_with(beginning)) {
            ret.push_back(item);
        }
    }

    return ret;
}

void AutoComplete::populate(IEnvironment &env) {
    _items = _source->list(env);
}
