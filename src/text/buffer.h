#pragma once

#include "meditfwd.h"
#include "text/fstring.h"
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

class Buffer {
public:
    using iterator = std::vector<FString>::iterator;

    Buffer() = default;
    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&) = default;
    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = default;

    Buffer(std::string text) {
        this->text(text);
    }

    Buffer(std::string_view text) : Buffer(std::string(text)) {}
    Buffer(const char *text) : Buffer(std::string{text}) {}

    [[nodiscard]] const auto &lines() const {
        return _lines;
    }

    [[nodiscard]] bool empty() const {
        return _lines.empty() || (_lines.size() == 0 && _lines.front().empty());
    }

    //! Get a line and trigger changed
    auto &lineAt(size_t index) {
        changed(true);
        return _lines.at(index);
    }

    //! Get a line without trigger changed
    const auto &lineAtConst(size_t index) const {
        return _lines.at(index);
    }

    void insert(size_t index, FString string) {
        if (_singleLine) {
            auto pos =
                (index == 0) ? _lines.front().begin() : _lines.front().end();
            _lines.front().insert(pos, string);
        }
        else {
            _lines.insert(_lines.begin() + index, std::move(string));
        }
        changed(true);
    }

    void push_back(FString string = {}) {
        _lines.push_back(std::move(string));
        changed(true);
    }

    void deleteLine(size_t l, size_t numLines = 1) {
        if (numLines == 0) {
            return;
        }
        else if (_lines.size() > 1) {
            _lines.erase(_lines.begin() + l, _lines.begin() + l + numLines);
        }
        else {
            _lines.front().clear();
        }
        changed(true);
    }

    auto changed() const {
        return _changed;
    }

    void changed(bool value) {
        _changed = value;
        if (value) {
            _oldColors = true;
        }
    }

    void oldColors(bool value) {
        _oldColors = value;
    }

    //! If the color needs updating
    bool oldColors() const {
        return _oldColors;
    }

    void clear() {
        _lines.clear();
        _lines.push_back({});
        changed(true);
    }

    Cursor begin();
    Cursor end();
    FChar front() const;
    FChar back() const;

    std::string text() const;
    void text(std::string);
    void text(std::string_view str) {
        text(std::string{str});
    }

    void text(std::istream &);
    void text(std::ostream &) const;

    void singleLine(bool value) {
        _singleLine = value;
    }

    bool singleLine() {
        return _singleLine;
    }

private:
    std::vector<FString> _lines = {""};
    bool _changed = false;
    bool _oldColors = false;
    bool _singleLine = false;
};
