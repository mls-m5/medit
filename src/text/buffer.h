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
class Buffer {
public:
    Buffer() = default;
    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    Buffer(bool isSingleLine) {
        _raw.singleLine(isSingleLine);
    }

    //    Buffer(std::string_view str, bool isSingleLine = false)
    //        : Buffer{FString{str}, isSingleLine} {}

    Buffer(const char *str, bool isSingleLine = false)
        : Buffer{FString{str}, isSingleLine} {}

    Buffer(FString str, bool isSingleLine = false)
        : _raw{str} {
        _raw.singleLine(isSingleLine);
    }

    History &history() {
        _tv();
        return _history;
    }

    [[nodiscard]] static std::unique_ptr<Buffer> open(
        std::filesystem::path path) {
        auto buffer = std::make_unique<Buffer>();

        auto file = std::make_unique<File>(path);
        if (std::filesystem::exists(file->path())) {
            file->load(buffer->_raw);
        }
        buffer->_file = std::move(file);
        return buffer;
    }

    void save() {
        _tv();
        if (_file) {
            _file->save(_raw);
            _raw.isChanged(false);
        }
    }

    void load() {
        _tv();
        if (_file) {
            _file->load(_raw);
            _raw.isChanged(false);
        }
    }

    [[nodiscard]] Path path() {
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

    [[nodiscard]] auto text() const {
        _tv();
        return _raw.text();
    }

    [[nodiscard]] const RawBuffer &raw() const {
        _tv();
        return _raw;
    }

    void isColorsOld(bool value) {
        _raw.isColorsOld(value);
    }

    [[nodiscard]] const bool isColorsOld() const {
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

    //! Get a line and trigger changed
    //    [[deprecated]] FString &lineAt(size_t index) {
    //        _tv();
    //        return _raw.lineAt(index);
    //    }

    void insert(size_t index, FString string) {
        _tv();
        _raw.insert(index, std::move(string));
    }

    void apply(BufferEdit edit) {
        _tv();
        _raw.apply(std::move(edit));
    }

    [[nodiscard]] const FString &lineAtConst(size_t index) const {
        return _raw.lineAtConst(index);
    }

    void pushBack(FString string = {}) {
        _raw.pushBack(string);
    }

    void deleteLine(size_t l, size_t numLines = 1) {
        _raw.deleteLine(l, numLines);
    }

    bool empty() const {
        return _raw.empty();
    }

    void text(std::string str) {
        _raw.text(std::move(str));
    }

    void text(std::string_view str) {
        _raw.text(std::string{str});
    }

    void text(std::istream &in) {
        _raw.text(in);
    }

    void text(std::ostream &out) const {
        _raw.text(out);
    }

    auto singleLine() const {
        return _raw.singleLine();
    }

    // Does not affect
    void format(const CursorRange &range, FormatType f) {
        _raw.format(range, f);
    }

    bool isChanged() const {
        return _raw.isChanged();
    }

    void isChanged(bool value) {
        _raw.isChanged(value);
    }

private:
    History _history{*this};
    std::unique_ptr<IFile> _file;
    Diagnostics _diagnostics;
    ThreadValidation _tv;

    RawBuffer _raw;
};
