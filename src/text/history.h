#pragma once

#include "text/bufferedit.h"
#include <string>
#include <vector>

class Buffer;

class History {
public:
    History(Buffer &buffer)
        : _buffer{buffer} {}

    struct Item {
        BufferEdit edit;
        long revision;

        void trim();
    };

    void commit();

    void undo();
    void redo();

    //! If you do not want changes to go back any longer
    //! example for when loading a file
    void clear();

    long revision() {
        return _revision;
    }

private:
    long _revision = 1;

    FString _currentState;
    Buffer &_buffer;

    std::vector<Item> _history;
    std::vector<Item> _redo;

    bool _ignoreRedoClear = false;
};
