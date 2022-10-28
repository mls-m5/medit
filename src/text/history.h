#pragma once

#include <string>
#include <vector>

class Buffer;

class History {
public:
    History(Buffer &buffer) : _buffer{buffer} {}

    struct Item {
        // Dummy implementation
        std::string old;
    };

    void commit();

    void undo();
    void redo();

    //! If you do not want changes to go back any longer
    //! example for when loading a file
    void clear();

private:
    std::string _currentState;
    Buffer &_buffer;

    std::vector<Item> _history;
    std::vector<Item> _redo;

    bool _ignoreRedoClear = false;
};
