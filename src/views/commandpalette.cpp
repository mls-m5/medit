#include "commandpalette.h"
#include "script/standardcommands.h"
#include "syntax/palette.h"
#include <algorithm>

CommandPalette::CommandPalette(IView *parent, StandardCommands &commands)
    : QuickList{parent, [this]() { return populate(); }}
    , _commands{commands} {}

QuickList::PopulateRetT CommandPalette::populate() const {
    auto ret = PopulateRetT{};
    for (auto &c : _commands.namedCommands) {
        auto str = c.first;
        std::replace(str.begin(), str.end(), '_', ' ');
        ret.push_back({
            .text = FString{str, Palette::identifier},
            .value = c.first,
            .emphasisStart = 0,
            .hint = {},
        });
    }
    // The commands are stored unordered; show them alphabetically.
    std::sort(ret.begin(), ret.end(), [](auto &a, auto &b) {
        return a.value < b.value;
    });
    return ret;
}

CommandPalette::~CommandPalette() = default;
