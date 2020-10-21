#pragma once

#include "cursor.h"
#include <string>
#include <string_view>
#include <vector>

class Buffer {

public:
    Buffer() = default;
    Buffer(const Buffer &) = default;
    Buffer(Buffer &&) = default;
    Buffer &operator=(const Buffer &) = default;
    Buffer &operator=(Buffer &&) = default;

    Buffer(std::string text) {
        this->text(text);
    }

    Buffer(std::string_view text) : Buffer(std::string(text)) {}

    const auto &lines() const {
        return _lines;
    }

    // Make sure that the cursor does not exeede the buffer
    Cursor fixCursor(Cursor cursor);
    Cursor prev(Cursor cursor);
    Cursor next(Cursor cursor); // todo: implement this

    Cursor insert(char c, Cursor cur);

    //! Remove the character after the cursor
    Cursor erase(Cursor cur);

    std::string text();
    void text(std::string);

private:
    std::vector<std::string> _lines;
};
