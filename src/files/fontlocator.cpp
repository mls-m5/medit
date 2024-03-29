#include "fontlocator.h"
#include "core/os.h"

namespace {

namespace filesystem = std::filesystem;

filesystem::path searchPath(filesystem::path path, filesystem::path name) {
    for (auto &it : filesystem::recursive_directory_iterator{path}) {
        if (it.path().filename().stem() == name) {
            return it.path();
        }
    }

    return {};
}

[[maybe_unused]] filesystem::path find(filesystem::path name) {
    if (auto exePath = executablePath(); !exePath.empty()) {
        if (auto path = searchPath(exePath.parent_path(), name);
            !path.empty()) {
            return path;
        }
    }

    if (auto path = searchPath("/usr/share/fonts", name); !path.empty()) {
        return path;
    }

    if (auto path = searchPath("/usr/local/share/fonts", name); !path.empty()) {
        return path;
    }

    if (auto env = getenv("HOME")) {
        auto fontPath = filesystem::path{env} / ".fonts/";

        if (filesystem::exists(fontPath)) {
            if (auto path = searchPath(fontPath, name); !path.empty()) {
                return path;
            }
        }
    }

    return {};
}

} // namespace

filesystem::path findFont(filesystem::path path) {
    if constexpr (getOs() == Os::Emscripten) {
        return "./data/UbuntuMono-Regular.ttf";
    }
    else {

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
}
