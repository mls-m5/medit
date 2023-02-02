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

    [[nodiscard]] const std::vector<FString> &lines() const;

    [[nodiscard]] bool empty() const;

    //! Get a line without trigger changed
    [[nodiscard]] const FString &lineAt(size_t index) const;

    [[nodiscard]] bool isChanged() const;

    void isChanged(bool value);

    // Allowed to be non-const because it does not change the actual text
    void isColorsOld(bool value);

    //! If the color needs updating
    [[nodiscard]] bool isColorsOld() const;

    void clear();

    /// Should be accompanied by _history.commit()
    void text(std::istream &);
    void text(std::ostream &) const;

    //! Get as a continuous string and not a vector of lines
    [[nodiscard]] FString ftext() const;

    [[nodiscard]] bool isSingleLine() const {
        return _isSingleLine;
    }

    void isSingleLine(bool value) {
        _isSingleLine = value;
    }

    /// Should be accompanied by _history.commit();
    Cursor apply(BufferEdit edit);

    void format(const CursorRange &range, FormatType f);

private:
    std::vector<FString> _lines = {""};
    bool _isSingleLine = false;
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    TimePoint _changedTime;
    TimePoint _formattedTime;
    TimePoint _savedTime;
};
