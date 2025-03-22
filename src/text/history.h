#pragma once

#include "text/bufferedit.h"
#include <vector>

class Buffer;

class History {
public:
    History(Buffer &buffer)
        : _buffer{&buffer} {}

    struct Item {
        BufferEdit edit;
        long revision = {};

        bool isMajor = false;

        void trim();
    };

    //! Add the last change to the history
    //! Replace this with a function that only takes the specific changes
    bool commit(BufferEdit edit);

    //! Remember that the last change was a change you might want to go back to
    void markMajor();

    void undoMajor();

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

    Buffer *_buffer = nullptr;

    std::vector<Item> _history;
    std::vector<Item> _redo;

    friend RawBuffer;
};
