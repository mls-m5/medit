#include "locator.h"
#include "meditfwd.h"
#include "quicklist.h"
#include "syntax/palette.h"
#include <algorithm>
#include <filesystem>

Locator::~Locator() = default;

Locator::Locator(IView *parent, Project &projectFiles)
    : QuickList{parent, [this]() { return populate(); }}
    , _project{projectFiles} {}

QuickList::PopulateRetT Locator::populate() const {
    // Always read new list so that we do not get an outdated
    // list
    const auto &root = _project.settings().root;
    auto ret = PopulateRetT{};
    for (auto &f : _project.files()) {
        // Project files lie below the root (the settings file may not).
        auto relative = f.lexically_relative(root);
        if (relative.empty() || *relative.begin() == "..") {
            relative = f;
        }
        const auto path = relative.generic_string();
        const auto slash = path.rfind('/');
        const auto nameStart = slash == std::string::npos ? 0 : slash + 1;

        auto text = FString{path.substr(0, nameStart), Palette::comment};
        text += FString{path.substr(nameStart), Palette::identifier};
        ret.push_back({
            .text = std::move(text),
            .value = path,
            .emphasisStart = FString{path.substr(0, nameStart)}.size(),
            .hint = {},
        });
    }
    // Before anything is typed: files closest to the root first.
    std::stable_sort(ret.begin(), ret.end(), [](auto &a, auto &b) {
        const auto depthA = std::count(a.value.begin(), a.value.end(), '/');
        const auto depthB = std::count(b.value.begin(), b.value.end(), '/');
        if (depthA != depthB) {
            return depthA < depthB;
        }
        return a.value < b.value;
    });
    return ret;
}
