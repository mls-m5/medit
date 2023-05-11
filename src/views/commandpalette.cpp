#include "commandpalette.h"
#include "script/standardcommands.h"
#include "syntax/palette.h"

CommandPalette::CommandPalette(IView *parent, StandardCommands &commands)
    : QuickList{parent, [this]() { return populate(); }}
    , _commands{commands} {}

QuickList::PopulateRetT CommandPalette::populate() const {
    auto ret = PopulateRetT{};
    for (auto &c : _commands.namedCommands) {
        //        auto path = std::filesystem::relative(f,
        //        _project.settings().root);
        ret.push_back({FString{c.first, Palette::identifier}, c.first});
    }
    return ret;
}

CommandPalette::~CommandPalette() = default;
