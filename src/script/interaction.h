#pragma once

#include "meditfwd.h"
#include "text/fstring.h"
#include "text/position.h"
#include <functional>
#include <memory>
#include <string>

/// Se `InteractionHandling` for the mechanics behind interactions
/// Also se `SimlpeInteraction` if you want a example or just a basic
/// interaction
struct Interaction {
    using InteractionCallback =
        std::function<void(std::shared_ptr<IEnvironment>, const Interaction &)>;

    FString text;
    Position cursorPosition;
    std::string title;

    /// Alternative syntax for opening a new interaction. It is possible that
    /// this is redundant. Well well...
    void begin(InteractionHandling &, InteractionCallback callback);

    std::string lineAt(size_t line) const;
    std::string lineAtCursor() const {
        return lineAt(cursorPosition.y());
    }
};
