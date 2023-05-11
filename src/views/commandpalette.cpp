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
        ret.push_back({FString{str, Palette::identifier}, c.first});
    }
    return ret;
}

CommandPalette::~CommandPalette() = default;
