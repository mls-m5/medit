#include "text/rawbuffer.h"
#include "cursorops.h"
#include "cursorrange.h"
#include "cursorrangeops.h"
#include "text/cursor.h"
#include <sstream>

std::string RawBuffer::text() const {
    std::ostringstream ss;

    text(ss);

    return ss.str();
}

void RawBuffer::text(std::string str) {
    std::istringstream ss{std::move(str)};

    text(ss);
}

void RawBuffer::text(std::istream &stream) {
    //    forceThread();
    isChanged(true);
    _lines.clear();

    for (std::string line; getline(stream, line);) {
        _lines.emplace_back(std::move(line));
    }
}

void RawBuffer::text(std::ostream &stream) const {
    //    forceThread();
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

void RawBuffer::apply(BufferEdit edit) {
    edit.trim();

    if (edit.empty()) {
        return;
    }

    auto pos = edit.position;

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

    auto lines = edit.to.split('\n');

    auto startStr = lineAt(edit.position.y()).substr(0, edit.position.x());
    auto endStr =
        lineAt(edit.position.y() + oldNumLines)
            .substr((oldNumLines ? 0 : edit.position.x()) + lastOldLineLength);

    lines.front().insert(lines.front().begin(), startStr);
    lines.back() += endStr;

    _lines.erase(_lines.begin() + edit.position.y(),
                 _lines.begin() + (edit.position.y() + oldNumLines + 1));

    _lines.insert(
        _lines.begin() + edit.position.y(), lines.begin(), lines.end());
}

void RawBuffer::format(const CursorRange &range, FormatType f) {
    // TODO: Implement multiline format
    if (range.end().x() < range.begin().x()) {
        return;
    }

    auto &line = lineAt(range.begin().y());

    for (size_t i = range.begin().x(); i < range.end().x(); ++i) {
        line.at(i).f = f;
    }
}
