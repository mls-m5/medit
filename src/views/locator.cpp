
#include "locator.h"
#include "meditfwd.h"
#include "quicklist.h"
#include "syntax/palette.h"
#include <filesystem>

Locator::~Locator() = default;

Locator::Locator(IView *parent, Project &projectFiles)
    : QuickList{parent, [this]() { return populate(); }}
    , _project{projectFiles} {}

QuickList::PopulateRetT Locator::populate() const {
    // Always read new list so that we do not get an outdated
    // list
    auto ret = PopulateRetT{};
    for (auto &f : _project.files()) {
        auto path = std::filesystem::relative(f, _project.settings().root);
        ret.push_back({FString{f.filename().string(), Palette::identifier},
                       path.string()});
    }
    return ret;
}
