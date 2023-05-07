#pragma once

#include "meditfwd.h"
#include <functional>
#include <memory>

/// The part of interaction handling that handles creating window
struct InteractionHandling {
    using InteractionCallback =
        std::function<void(std::shared_ptr<IEnvironment>, const Interaction &)>;

    MainWindow &_window;
    std::weak_ptr<Buffer> _operationBuffer;
    InteractionCallback _callback;
    std::weak_ptr<Editor> _editor;

    //! Create interaction buffer and provide function to be called when
    //! the interaction is accepted, also let the editor handling waiting for
    //! the user to provide feedback
    void newInteraction(const Interaction &, InteractionCallback);

    bool keyPress(std::shared_ptr<IEnvironment>);

    /// Reset interaction and close buffer
    void close();
};
