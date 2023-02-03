#include "text/rawbuffer.h"
#include "cursorops.h"
#include "cursorrange.h"
#include "cursorrangeops.h"
#include "text/cursor.h"
#include <sstream>

const std::vector<FString> &RawBuffer::lines() const {
    return _lines;
}

bool RawBuffer::empty() const {
    return _lines.empty() || (_lines.size() == 0 && _lines.front().empty());
}

const FString &RawBuffer::lineAt(size_t index) const {
    return _lines.at(index);
}

bool RawBuffer::isChanged() const {
    return _changedTime > _savedTime;
}

void RawBuffer::isChanged(bool value) {
    if (value) {
        _changedTime = std::chrono::high_resolution_clock::now();
    }
    else {
        _savedTime = std::chrono::high_resolution_clock::now();
    }
}

void RawBuffer::isColorsOld(bool value) {
    if (value) {
    }
    else {
        _formattedTime = std::chrono::high_resolution_clock ::now();
    }
}

bool RawBuffer::isColorsOld() const {
    return _formattedTime < _changedTime;
}

void RawBuffer::clear() {
    _lines.clear();
    _lines.push_back({});
    isChanged(true);
}

// void RawBuffer::text(std::istream &stream) {
//     isChanged(true);
//     _lines.clear();

//    for (std::string line; getline(stream, line);) {
//        _lines.emplace_back(std::move(line));
//    }
//}

void RawBuffer::text(std::ostream &stream) const {
    if (_lines.empty()) {
        return;
    }

    stream << std::string{_lines.front()};
    for (size_t i = 1; i < _lines.size(); ++i) {
        stream << "\n" << std::string{_lines.at(i)};
    }
}

FString RawBuffer::ftext() const {
    return FString::join(_lines, '\n');
}

Cursor RawBuffer::apply(BufferEdit edit) {
    edit.trim();

    if (edit.empty()) {
        return edit.position;
    }

    const auto pos = edit.position;

    auto &from = edit.from;

    auto oldNumLines = std::count(from.begin(), from.end(), Utf8Char{'\n'});

    auto lastOldLineLength = [oldNumLines, &from] {
        if (oldNumLines) {
            if (from.back() == Utf8Char{'\n'}) {
                return size_t{0};
            }
            for (size_t i = from.size() - 1; i != 0; --i) {
                if (from.at(i) == Utf8Char{'\n'}) {
                    return from.size() - i - 1;
                }
            }
        }
        return from.size();
    }();

    auto newLines = edit.to.split('\n');

    auto startStr = _lines.at(edit.position.y()).substr(0, edit.position.x());
    auto endStr =
        _lines.at(edit.position.y() + oldNumLines)
            .substr((oldNumLines ? 0 : edit.position.x()) + lastOldLineLength);

    auto ret = edit.position;
    ret.y(ret.y() + _lines.size() - oldNumLines);
    if (newLines.size() == 1) {
        ret.x(pos.x() + newLines.back().size());
    }
    else {
        ret.x(newLines.back().size());
    }

    newLines.front().insert(newLines.front().begin(), startStr);
    newLines.back() += endStr;

    _lines.erase(_lines.begin() + edit.position.y(),
                 _lines.begin() + (edit.position.y() + oldNumLines + 1));

    _lines.insert(
        _lines.begin() + edit.position.y(), newLines.begin(), newLines.end());

    isColorsOld(true);
    isChanged(true);

    return ret;
}

void RawBuffer::format(CursorRange range, FormatType f) {
    range = fix(range);

    if (range.end().x() < range.begin().x()) {
        return;
    }

    if (range.end().y() == range.begin().y()) {
        auto &line = _lines.at(range.begin().y());

        for (size_t i = range.begin().x(); i < range.end().x(); ++i) {
            line.at(i).f = f;
        }
        return;
    }

    {
        auto &line1 = _lines.at(range.begin().y());
        for (size_t i = range.begin().x(); i < line1.size(); ++i) {
            line1.at(i).f = f;
        }
    }
    {
        auto &line2 = _lines.at(range.end().y());
        for (size_t i = 0; i < range.end().x(); ++i) {
            line2.at(i).f = f;
        }
    }
}
