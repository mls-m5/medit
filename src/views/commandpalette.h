#pragma once

#include "quicklist.h"
#include "script/standardcommands.h"

class CommandPalette : public QuickList {
public:
    CommandPalette(IView *parent, StandardCommands &commands);
    ~CommandPalette() override;

private:
    StandardCommands &_commands;
    PopulateRetT populate() const;
};
