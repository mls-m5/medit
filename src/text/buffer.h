#pragma once

#include "text/cursor.h"
#include "text/fstring.h"
#include <iosfwd>
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

    [[nodiscard]] const auto &lines() const {
        return _lines;
    }

    [[nodiscard]] auto &lines() {
        return _lines;
    }

    Cursor insert(Utf8Char c, Cursor cur);

    //! Remove the character before the cursor
    Cursor erase(Cursor cur);

    std::string text() const;
    void text(std::string);

    void text(std::istream &);
    void text(std::ostream &) const;

private:
    std::vector<FString> _lines;
};
