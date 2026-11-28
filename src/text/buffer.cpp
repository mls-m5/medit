#include "buffer.h"
#include "cursorops.h"
#include "cursorrange.h"
#include "cursorrangeops.h"
#include "files/file.h"
#include <filesystem>
#include <sstream>
#include <stdexcept>

std::unique_ptr<Buffer> Buffer::open(std::filesystem::path path) {
    auto buffer = std::make_unique<Buffer>();

    auto file = std::make_unique<File>(path);
    if (std::filesystem::is_regular_file(file->path())) {
        file->load(*buffer);
        buffer->_file = std::move(file);
        // The text before loading is not something to undo to.
        buffer->_history.clear();
        buffer->_history.event(HistoryKind::Load);
    }
    return buffer;
}

bool Buffer::save(bool overwriteDiskChanges) {
    _tv();
    if (!_file) {
        return false;
    }
    // A file deleted on disk is written again; only someone else's content
    // is protected.
    if (!overwriteDiskChanges &&
        _file->diskState() == DiskState::ChangedOnDisk) {
        return false;
    }
    if (_file->save(*this)) {
        _raw.isChanged(false);
        diskState(DiskState::InSync);
        _history.event(HistoryKind::Save);
        return true;
    }
    return false;
}

void Buffer::assignFile(std::unique_ptr<IFile> file) {
    _tv();
    if (_file) {
        throw std::runtime_error{
            "trying to assign file to buffer that already has file"};
    }
    _file = std::move(file);
}

void Buffer::load() {
    _tv();
    if (_file) {
        if (auto bytes = _file->readDisk()) {
            reload(*bytes);
        }
    }
}

void Buffer::reload(const std::string &bytes) {
    _tv();
    if (!_file) {
        return;
    }
    const auto revision = _history.revision();
    replaceText(bytes);
    _history.tagSince(revision, HistoryKind::Reload);
    _file->loaded(bytes);
    _history.event(HistoryKind::Load);
    _raw.isChanged(false);
    diskState(DiskState::InSync);
    emitChangeSignal();
}

void Buffer::replaceText(std::string_view text) {
    _tv();
    const auto current = this->text();
    const auto from = splitLines(current);
    const auto to = splitLines(text);
    const auto hunks = diffLines(from, to);
    if (hunks.empty()) {
        return;
    }

    auto joined = [&to](size_t begin, size_t end) {
        auto ret = std::string{};
        for (auto i = begin; i < end; ++i) {
            if (i > begin) {
                ret += '\n';
            }
            ret += to[i];
        }
        return ret;
    };

    _history.markMajor();
    // From the last hunk, so the line numbers of earlier hunks stay valid.
    for (auto it = hunks.rbegin(); it != hunks.rend(); ++it) {
        const auto lineCount = lines().size();
        const auto &h = *it;
        const auto hasNew = h.toBegin < h.toEnd;
        auto replacement = joined(h.toBegin, h.toEnd);
        auto range = CursorRange{*this};
        if (h.fromEnd < lineCount) {
            // Whole lines including their newlines.
            range = CursorRange{*this, {0, h.fromBegin}, {0, h.fromEnd}};
            if (hasNew) {
                replacement += '\n';
            }
        }
        else if (h.fromBegin > 0) {
            // Up to the end: take the newline before the range instead.
            const auto before = h.fromBegin - 1;
            range = CursorRange{*this,
                                {lines().at(before).size(), before},
                                {lines().back().size(), lineCount - 1}};
            if (hasNew) {
                replacement.insert(0, "\n");
            }
        }
        // Otherwise the whole buffer, which `range` already is.
        replace(range, FString{replacement});
    }
    _history.markMajor();
}

void Buffer::diskState(DiskState state, std::vector<LineRange> conflicts) {
    _tv();
    _diskState = state;
    if (state == DiskState::InSync) {
        _diagnostics.clearSource("disk");
        return;
    }

    constexpr size_t maxMarkedLines = 200;
    const auto message = std::string{
        state == DiskState::DeletedOnDisk ? "deleted on disk"
        : conflicts.empty()               ? "changed on disk"
                            : "changed on disk: conflicts with your edit"};
    if (conflicts.empty()) {
        conflicts.push_back({0, 1});
    }
    auto marks = std::vector<Diagnostics::Diagnostic>{};
    const auto last = lines().size() - 1;
    for (auto &range : conflicts) {
        // An empty range (a deleted line) is shown on the line after it.
        const auto end = std::max(range.end, range.begin + 1);
        for (auto line = range.begin;
             line < end && marks.size() < maxMarkedLines;
             ++line) {
            const auto y = std::min(line, last);
            marks.push_back({
                .type = DiagnosticType::Error,
                .source = "disk",
                .message = message,
                .range = {{0, y}, {0, y}},
            });
        }
    }
    _diagnostics.publish("disk", std::move(marks));
}

std::string Buffer::text() const {
    std::ostringstream ss;

    text(ss);

    return ss.str();
}

void Buffer::clear() {
    replace(all(*this), "");
}

Cursor Buffer::apply(BufferEdit edit) {
    _tv();
    _history.commit(edit);
    auto cur = applyRaw(std::move(edit));
    emitChangeSignal();
    return cur;
}

Cursor Buffer::applyRaw(BufferEdit edit) {
    if (_anchors.empty()) {
        return _raw.apply(std::move(edit));
    }
    edit.trim();
    for (auto &anchor : _anchors) {
        anchor.position = movePosition(anchor.position, edit);
    }
    return _raw.apply(std::move(edit));
}

size_t Buffer::addAnchor(Position position) {
    _tv();
    const auto id = _nextAnchorId++;
    _anchors.push_back({id, position});
    return id;
}

void Buffer::removeAnchor(size_t id) {
    _tv();
    std::erase_if(_anchors, [id](const Anchor &a) { return a.id == id; });
}

Position Buffer::anchor(size_t id) const {
    _tv();
    for (auto &a : _anchors) {
        if (a.id == id) {
            return a.position;
        }
    }
    return {};
}

void Buffer::anchor(size_t id, Position position) {
    _tv();
    for (auto &a : _anchors) {
        if (a.id == id) {
            a.position = position;
            return;
        }
    }
}

FStringView Buffer::lineAt(size_t index) const {
    _tv();
    return _raw.lineAt(index);
}

void Buffer::pushBack(FString string) {
    _tv();
    string.insert(0, '\n');
    insert(end(), std::move(string));
}

void Buffer::text(std::string str) {
    _tv();
    std::istringstream ss{std::move(str)};

    text(ss);
}

void Buffer::text(std::string_view str) {
    text(std::string{str});
}

void Buffer::text(std::istream &in) {
    auto ss = std::ostringstream{};

    ss << in.rdbuf();

    auto fstr = FString{ss.str()};

    replace(all(*this), std::move(fstr));
}

void Buffer::text(std::ostream &out) const {
    _tv();
    _raw.text(out);
}

void Buffer::format(const CursorRange &range, FormatType f) {
    _tv();
    _raw.format(range, f);
}
