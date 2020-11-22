#pragma once

#include "text/buffer.h"
#include <vector>

class History {
    Buffer _currentState;

public:
    struct Item {
        //        size_t position;
        //        std::vector<std::string> lines;

        // Dummy implementation
        std::string old;
    };

    std::vector<Item> _history;

    void commit(const Buffer &buffer);

    void undo(Buffer &buffer);

    //! If you do not want changes to go back any longer
    //! example for when loading a file
    void clear();
};
