#pragma once

#include "meditfwd.h"
#include "text/fstring.h"
#include <chrono>
#include <iosfwd>
#include <string>
#include <string_view>
#include <thread>
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
        forceThread();
        return _lines;
    }

    [[nodiscard]] bool empty() const {
        forceThread();
        return _lines.empty() || (_lines.size() == 0 && _lines.front().empty());
    }

    //! Get a line and trigger changed
    auto &lineAt(size_t index) {
        forceThread();
        isChanged(true);
        return _lines.at(index);
    }

    //! Get a line without trigger changed
    const auto &lineAtConst(size_t index) const {
        forceThread();
        return _lines.at(index);
    }

    void insert(size_t index, FString string) {
        forceThread();
        if (_singleLine) {
            auto pos =
                (index == 0) ? _lines.front().begin() : _lines.front().end();
            _lines.front().insert(pos, string);
        }
        else {
            _lines.insert(_lines.begin() + index, std::move(string));
        }
        isChanged(true);
    }

    void push_back(FString string = {}) {
        forceThread();
        _lines.push_back(std::move(string));
        isChanged(true);
    }

    void deleteLine(size_t l, size_t numLines = 1) {
        forceThread();
        if (numLines == 0) {
            return;
        }
        else if (_lines.size() > 1) {
            _lines.erase(_lines.begin() + l, _lines.begin() + l + numLines);
        }
        else {
            _lines.front().clear();
        }
        isChanged(true);
    }

    auto isChanged() const {
        return _changedTime > _savedTime;
    }

    void isChanged(bool value) {
        //        _changed = value;
        if (value) {
            _changedTime = std::chrono::high_resolution_clock::now();
            //            _oldColors = true;
        }
        else {
            _savedTime = std::chrono::high_resolution_clock::now();
        }
    }

    void isColorsOld(bool value) {
        if (value) {
        }
        else {
            _formattedTime = std::chrono::high_resolution_clock ::now();
        }
    }

    //! If the color needs updating
    bool isColorsOld() const {
        return _formattedTime < _changedTime;
    }

    void clear() {
        forceThread();
        _lines.clear();
        _lines.push_back({});
        //        _lines = {{}};
        isChanged(true);
    }

    //! Create a cursor from a position
    Cursor cursor(Position pos);

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

    friend std::ostream &operator<<(std::ostream &stream,
                                    const Buffer &buffer) {
        buffer.text(stream);
        return stream;
    }

    friend std::istream &operator>>(std::istream &stream, Buffer &buffer) {
        buffer.text(stream);
        return stream;
    }

    void singleLine(bool value) {
        _singleLine = value;
    }

    bool singleLine() {
        return _singleLine;
    }

    void forceThread() const {
        if (std::this_thread::get_id() != _threadId) {
            throw std::runtime_error(
                "buffer called from another thread than the one starded from");
        }
    }

private:
    std::vector<FString> _lines = {""};
    //    bool _changed = false;
    //    bool _oldColors = false;
    bool _singleLine = false;
    std::thread::id _threadId = std::this_thread::get_id();

    using TimePoint = std::chrono::high_resolution_clock::time_point;
    TimePoint _changedTime;
    TimePoint _formattedTime;
    TimePoint _savedTime;
};
