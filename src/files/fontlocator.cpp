#include "fontlocator.h"
#include <iostream>

namespace {

filesystem::path find(filesystem::path name) {
    for (auto &it :
         filesystem::recursive_directory_iterator{"/usr/share/fonts"}) {
        if (it.path().filename().stem() == name) {
            std::cout << it.path() << std::endl;
            return it.path();
        }
    }

    for (auto &it :
         filesystem::recursive_directory_iterator{"/usr/local/share/fonts"}) {
        if (it.path().filename().stem() == name) {
            return it.path();
        }
    }

    if (auto env = getenv("HOME")) {
        auto fontPath = filesystem::path{env} / ".fonts/";
        if (filesystem::exists(fontPath)) {
            for (auto &it :
                 filesystem::recursive_directory_iterator{fontPath}) {
                if (it.path().filename().stem() == name) {
                    return it.path();
                }
            }
        }
    }
    return {};
}

} // namespace

filesystem::path findFont(filesystem::path path) {
    auto replaced = [&] {
        auto replaced = path.string();
        for (auto &c : replaced) {
            if (c == ' ') {
                c = '-';
            }
        }
        return filesystem::path{replaced};
    }();

    if (auto f = find(replaced); !f.empty()) {
        return f;
    }

    if (auto f = find(path); !f.empty()) {
        return f;
    }

    return {};
}
