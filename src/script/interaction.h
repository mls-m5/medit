#pragma once

#include "text/fstring.h"
#include "text/position.h"
#include <string>

/// Se `InteractionHandling` for the mechanics behind interactions
/// Also se `SimlpeInteraction` if you want a example or just a basic
/// interaction
struct Interaction {
    FString text;
    Position cursorPosition;
    std::string title;
};
