#pragma once

#include "quicklist.h"
#include "script/standardcommands.h"

class CommandPalette : public QuickList {
public:
    CommandPalette(const CommandPalette &) = delete;
    CommandPalette(CommandPalette &&) = delete;
    CommandPalette &operator=(const CommandPalette &) = delete;
    CommandPalette &operator=(CommandPalette &&) = delete;

    CommandPalette(IView *parent, StandardCommands &commands);
    ~CommandPalette() override;

private:
    StandardCommands &_commands;
    PopulateRetT populate() const;
};
