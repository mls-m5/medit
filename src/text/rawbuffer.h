#include "cursor.h"
#include "fstring.h"
#include <chrono>
#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#pragma once

// Handle the bare essentials of a file in memory
// It is guaranteed to have at least one (empty) line at all time
// For more functionality use `Buffer`
class RawBuffer {
public:
    using iterator = std::vector<FString>::iterator;

    RawBuffer() = default;
    RawBuffer(const RawBuffer &) = delete;
    RawBuffer(RawBuffer &&) = delete;
    RawBuffer &operator=(const RawBuffer &) = delete;
    RawBuffer &operator=(RawBuffer &&) = delete;

    RawBuffer(std::string text) {
        this->text(text);
    }

    RawBuffer(std::string_view text)
        : RawBuffer(std::string(text)) {}
    RawBuffer(const char *text)
        : RawBuffer(std::string{text}) {}

    void copyFrom(const RawBuffer &buffer) {
        //        forceThread();

        _lines = buffer.lines();
        _singleLine = buffer._singleLine;

        isChanged(true);
    }

    [[nodiscard]] const std::vector<FString> &lines() const {
        //        forceThread();
        return _lines;
    }

    [[nodiscard]] bool empty() const {
        //        forceThread();
        return _lines.empty() || (_lines.size() == 0 && _lines.front().empty());
    }

    //! Get a line and trigger changed
    [[deprecated]] FString &lineAt(size_t index) {
        //        forceThread();
        isChanged(true);
        return _lines.at(index);
    }

    //! Get a line without trigger changed
    const FString &lineAtConst(size_t index) const {
        //        forceThread();
        return _lines.at(index);
    }

    //    [[deprecated]] void insert(size_t index, FString string) {
    //        //        forceThread();
    //        if (_singleLine) {
    //            auto pos =
    //                (index == 0) ? _lines.front().begin() :
    //                _lines.front().end();
    //            _lines.front().insert(pos, std::move(string));
    //        }
    //        else {
    //            _lines.insert(_lines.begin() + index, std::move(string));
    //        }
    //        isChanged(true);
    //    }

    [[deprecated]] void pushBack(FString string = {}) {
        //        forceThread();
        _lines.push_back(std::move(string));
        isChanged(true);
    }

    [[deprecated]] void deleteLine(size_t l, size_t numLines = 1) {
        //        forceThread();
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

    bool isChanged() const {
        return _changedTime > _savedTime;
    }

    void isChanged(bool value) {
        if (value) {
            _changedTime = std::chrono::high_resolution_clock::now();
        }
        else {
            _savedTime = std::chrono::high_resolution_clock::now();
        }
    }

    // Allowed to be non-const because it does not change the actual text
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
        //        forceThread();
        _lines.clear();
        _lines.push_back({});
        isChanged(true);
    }

    std::string text() const;
    [[deprecated]] void text(std::string);
    [[deprecated]] void text(std::string_view str) {
        text(std::string{str});
    }

    [[deprecated]] void text(std::istream &);
    [[deprecated]] void text(std::ostream &) const;

    //! Get as a continuous string and not a vector of lines
    FString ftext() const;

    [[deprecated]] friend std::ostream &operator<<(std::ostream &stream,
                                                   const RawBuffer &buffer) {
        buffer.text(stream);
        return stream;
    }

    [[deprecated]] friend std::istream &operator>>(std::istream &stream,
                                                   RawBuffer &buffer) {
        buffer.text(stream);
        return stream;
    }

    bool singleLine() const {
        return _singleLine;
    }

    Cursor apply(BufferEdit edit);

    void format(const CursorRange &range, FormatType f);

    void singleLine(bool value) {
        _singleLine = value;
    }

private:
    std::vector<FString> _lines = {""};
    bool _singleLine = false;
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    TimePoint _changedTime;
    TimePoint _formattedTime;
    TimePoint _savedTime;

    //    friend Buffer;
};
