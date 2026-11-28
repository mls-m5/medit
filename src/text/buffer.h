#pragma once

#include "core/threadvalidation.h"
#include "files/ifile.h"
#include "files/ifilesystem.h"
#include "files/path.h"
#include "history.h"
#include "rawbuffer.h"
#include "text/diagnostics.h"
#include "text/fstringview.h"
#include "text/history.h"
#include "text/linemerge.h"
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

// Text stored in memory with assosiated undo/redo-history and associated
// filename information
class Buffer : public std::enable_shared_from_this<Buffer> {
public:
    Buffer() = default;
    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&) = delete;
    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;
    ~Buffer() = default;

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

    /// A buffer with the file at `path`, loaded if it is a regular file.
    [[nodiscard]] static std::unique_ptr<Buffer> open(
        std::filesystem::path path,
        IFileSystem &fileSystem = localFileSystem());

    /// Writes the buffer to its file. Unless `overwriteDiskChanges`, refuses
    /// (returns false and writes nothing) when someone else changed the file
    /// since it was last loaded or saved. A deleted file is written again.
    bool save(bool overwriteDiskChanges = false);

    /// Used when saving a buffer that is not saved yet. Check usage in the
    /// project to se how it is used
    void assignFile(std::unique_ptr<IFile> file);

    /// Reload from buffers used file. Only the changed lines are replaced,
    /// as one undo step. Discards unsaved changes. Nothing happens if the file
    /// cannot be read.
    void load();

    /// As load(), with the file's content already read.
    void reload(const std::string &bytes);

    /// Takes in a change made on disk that was merged with the unsaved
    /// changes: `text` is the merged text and `diskBytes` what the file now
    /// contains. One undo step, recorded as a merge; the buffer stays unsaved.
    void mergeFromDisk(std::string_view text, const std::string &diskBytes);

    /// Replaces the text with `text`, changing only the lines that differ, as
    /// one undo step.
    void replaceText(std::string_view text);

    [[nodiscard]] DiskState diskState() const {
        _tv();
        return _diskState;
    }

    /// Marks how the file on disk relates to the buffer. `conflicts` are the
    /// lines shown as conflicting (with the "disk" diagnostics source); they
    /// are cleared when the state is `InSync`.
    void diskState(DiskState state, std::vector<LineRange> conflicts = {});

    /// Positions that every edit moves along with the text (see
    /// TrackedPosition, which manages them). Ids are never 0.
    size_t addAnchor(Position position);
    void removeAnchor(size_t id);
    [[nodiscard]] Position anchor(size_t id) const;
    void anchor(size_t id, Position position);

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
            throw std::out_of_range("RawBuffer is empty when calling front()");
        }
        return lines.front().front();
    }

    [[nodiscard]] FChar back() const {
        _tv();
        auto &lines = _raw.lines();
        if (lines.empty() || lines.back().empty()) {
            throw std::out_of_range("RawBuffer is empty when calling back()");
        }
        return lines.back().back();
    }

    void clear();

    [[nodiscard]] const std::vector<FString> &lines() const {
        _tv();
        return _raw.lines();
    }

    //! Apply changes and apply to history
    Cursor apply(BufferEdit edit);

    [[nodiscard]] FStringView lineAt(size_t index) const;

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

    /// Subscribe to changes
    void subscribe(std::function<void()> f, void *ref) {
        _tv();

        _subscribers.push_back({f, ref});
    }

    void unsubscribe(void *ref) {
        auto it = std::remove_if(_subscribers.begin(),
                                 _subscribers.end(),
                                 [ref](auto &s) { return s.ref == ref; });

        _subscribers.erase(it, _subscribers.end());
    }

    void emitChangeSignal() const {
        _tv();
        for (auto &sub : _subscribers) {
            sub.f();
        }
    }

private:
    // The function history class uses with undo/redo
    void applyWithoutHistory(BufferEdit edit) {
        _tv();
        applyRaw(std::move(edit));
        emitChangeSignal();
    }

    /// Applies the edit to the text and moves the anchors.
    Cursor applyRaw(BufferEdit edit);

    History _history{*this};
    std::unique_ptr<IFile> _file;
    Diagnostics _diagnostics;
    DiskState _diskState = DiskState::InSync;

    struct Anchor {
        size_t id = 0;
        Position position;
    };
    std::vector<Anchor> _anchors;
    size_t _nextAnchorId = 1;
    ThreadValidation _tv{"gui thread"};

    RawBuffer _raw;

    friend History;

    struct Subscriber {
        std::function<void()> f;
        void *ref;
    };

    std::vector<Subscriber> _subscribers;
};
