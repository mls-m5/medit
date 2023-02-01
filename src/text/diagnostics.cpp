#include "diagnostics.h"
#include <algorithm>

void Diagnostics::clearSource(std::string_view name) {
    auto it = std::remove_if(_list.begin(), _list.end(), [name](auto &d) {
        return d.source == name;
    });

    _list.erase(it, _list.end());
}

void Diagnostics::publish(std::string source, std::vector<Diagnostic> data) {
    clearSource(source);
    for (auto &item : data) {
        _list.push_back(std::move(item));
    }

    std::sort(_list.begin(), _list.end(), [](auto &&a, auto &&b) {
        return a.range.begin.y() < b.range.begin.y();
    });
}

bool Diagnostics::hasLineDiagnostic(int line) {
    for (auto &item : _list) {
        if (item.range.begin.y() == line)
            return true;
    }
    return false;
}
