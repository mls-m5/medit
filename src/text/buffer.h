#pragma once

#include "core/threadvalidation.h"
#include "files/file.h"
#include "files/path.h"
#include "rawbuffer.h"
#include "text/diagnostics.h"
#include "text/history.h"
#include <filesystem>
#include <memory>

// Text stored in memory with assosiated undo/redo-history and associated
// filename information
class Buffer : public std::enable_shared_from_this<Buffer> {
public:
    Buffer() = default;
    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    Buffer(bool isSingleLine) {
        _raw.isSingleLine(isSingleLine);
    }

    Buffer(const char *str, bool isSingleLine = false)
        : Buffer{FString{str}, isSingleLine} {}

    Buffer(FString str, bool isSingleLine = false) {
        text(str);
        _raw.isSingleLine(isSingleLine);
        _history.markMajor();
    }

    History &history() {
        _tv();
        return _history;
    }

    [[nodiscard]] static std::unique_ptr<Buffer> open(
        std::filesystem::path path);

    void save();

    /// Reload from buffers used file
    void load();

    [[nodiscard]] Path path() {
        _tv();
        if (_file) {
            return _file->path();
        }
        else {
            return {};
        }
    }

    [[nodiscard]] auto *file() {
        _tv();
        return _file.get();
    }

    [[nodiscard]] Diagnostics &diagnostics() {
        _tv();
        return _diagnostics;
    }

    [[nodiscard]] auto ftext() const {
        _tv();
        return _raw.ftext();
    }

    /// Get content as std::string
    [[nodiscard]] std::string text() const;

    [[nodiscard]] const RawBuffer &raw() const {
        _tv();
        return _raw;
    }

    void isColorsOld(bool value) {
        _tv();
        _raw.isColorsOld(value);
    }

    [[nodiscard]] bool isColorsOld() const {
        _tv();
        return _raw.isColorsOld();
    }

    //! Create a cursor from a position
    [[nodiscard]] Cursor cursor(Position pos) {
        _tv();
        return {*this, pos};
    }

    [[nodiscard]] Cursor begin() {
        _tv();
        return {*this};
    }

    [[nodiscard]] Cursor end() {
        _tv();
        auto &lines = _raw.lines();
        return {*this, lines.back().size(), lines.size() - 1};
    }

    [[nodiscard]] FChar front() const {
        _tv();
        auto &lines = _raw.lines();
        if (lines.empty() || lines.front().empty()) {
            std::out_of_range("RawBuffer is empty when calling front()");
        }
        return lines.front().front();
    }

    [[nodiscard]] FChar back() const {
        _tv();
        auto &lines = _raw.lines();
        if (lines.empty() || lines.front().empty()) {
            std::out_of_range("RawBuffer is empty when calling back()");
        }
        return lines.back().back();
    }

    void clear() {
        _tv();
        // TODO: create applystatement
        _raw.clear();
    }

    [[nodiscard]] const std::vector<FString> &lines() const {
        _tv();
        return _raw.lines();
    }

    //! Apply changes and apply to history
    Cursor apply(BufferEdit edit);

    [[nodiscard]] const FString &lineAt(size_t index) const;

    void pushBack(FString string = {});

    [[nodiscard]] bool empty() const {
        _tv();
        return _raw.empty();
    }

    void text(std::string str);

    void text(std::string_view str);

    void text(std::istream &in);

    void text(std::ostream &out) const;

    [[nodiscard]] bool isSingleLine() const {
        _tv();
        return _raw.isSingleLine();
    }

    void format(const CursorRange &range, FormatType f);

    [[nodiscard]] bool isChanged() const {
        _tv();
        return _raw.isChanged();
    }

    void isChanged(bool value) {
        _tv();
        _raw.isChanged(value);
    }

    friend std::ostream &operator<<(std::ostream &stream,
                                    const Buffer &buffer) {
        buffer.text(stream);
        return stream;
    }

    friend std::istream &operator>>(std::istream &stream, Buffer &buffer) {
        buffer.text(stream);
        return stream;
    }

private:
    // Function history class uses
    void applyWithoutHistory(BufferEdit edit) {
        _tv();
        _raw.apply(std::move(edit));
    }

    History _history{*this};
    std::unique_ptr<IFile> _file;
    Diagnostics _diagnostics;
    ThreadValidation _tv{"gui thread"};

    RawBuffer _raw;

    friend History;
};
