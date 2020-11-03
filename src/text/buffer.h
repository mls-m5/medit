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

    Buffer(std::string_view text)
        : Buffer(std::string(text)){}

              [[nodiscard]] const auto &
          lines() const {
        return _lines;
    }

    auto &lineAt(size_t index) {
        _changed = true;
        return _lines.at(index);
    }

    const auto &lineAt(size_t index) const {
        return _lines.at(index);
    }

    void insert(size_t index, FString string) {
        _lines.insert(_lines.begin() + index, std::move(string));
        changed(true);
    }

    void insert(std::vector<FString>::iterator position, FString string) {
        _lines.insert(position, std::move(string));
        changed(true);
    }

    void push_back(FString string = {}) {
        _lines.push_back(std::move(string));
        changed(true);
    }

    void deleteLine(size_t l) {
        _lines.erase(_lines.begin() + l);
        changed(true);
    }

    auto changed() const {
        return _changed;
    }

    void changed(bool value) {
        _changed = value;
    }

    void clear() {
        _lines.clear();
        _changed = true;
    }

    std::string text() const;
    void text(std::string);

    void text(std::istream &);
    void text(std::ostream &) const;

private:
    std::vector<FString> _lines;
    bool _changed = false;
};
