#pragma once

#include "meditfwd.h"
#include <algorithm>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

struct Interaction {
    std::string op;
    std::vector<std::pair<std::string, std::string>> values;

    bool valid = true;

    /// Create text form
    void serialize(std::ostream &os) const;

    /// Parse text form
    void deserialize(std::istream &is);

    using InteractionCallback =
        std::function<void(std::shared_ptr<IEnvironment>, Interaction &)>;

    //! Create interaction buffer and provide function to be called when
    //! the interaction is accepted, also let the editor handling waiting for
    //! the user to provide feedback
    void handle(std::shared_ptr<IEnvironment> env, InteractionCallback);
};
