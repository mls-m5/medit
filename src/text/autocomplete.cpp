

#include "autocomplete.h"

AutoComplete::AutoComplete() {
    _items.push_back({"apa", "a class"});
    _items.push_back({"bepa", "a function"});
}

AutoComplete::~AutoComplete() {}

AutoComplete::CompletionList AutoComplete::getMatching(std::string beginning) {
    CompletionList ret;

    for (auto &item : _items) {
        if (item.name.find(beginning) != std::string::npos) {
            ret.push_back(item);
        }
    }

    return ret;
}

void AutoComplete::populate(AutoComplete::CompletionList items) {
    _items = items;
}
