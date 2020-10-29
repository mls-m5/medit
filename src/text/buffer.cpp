#include "text/buffer.h"
#include "text/colorize.h"
#include <sstream>

Cursor Buffer::insert(Utf8Char c, Cursor cur) {
    if (_lines.empty()) {
        _lines.emplace_back();
    }
    cur = fix(cur);
    if (c == '\n') {
        _lines.insert(_lines.begin() + cur.y() + 1, FString{});
        cur.y() += 1;
        cur.x() = 0;
    }
    else {
        auto &line = _lines.at(cur.y());
        line.insert(cur.x(), c);
        colorize(line);
        cur.x() += 1;
    }

    return cur;
}

Cursor Buffer::erase(Cursor cur) {
    if (_lines.empty()) {
        return {*this};
    }
    else if (_lines.size() == 1 && _lines.front().empty()) {
        return {*this};
    }
    cur = fix(cur);
    auto &line = _lines.at(cur.y());
    if (cur.x() == 0) {
        if (cur.y() > 0) {
            auto oldLine = std::move(_lines.at(cur.y()));
            _lines.erase(_lines.begin() + cur.y());
            cur.y() -= 1;
            auto &lineAbove = _lines.at(cur.y());
            cur.x() = lineAbove.size();
            lineAbove += oldLine;
            colorize(lineAbove);
        }
    }
    else {
        line.erase(cur.x() - 1, 1);
        colorize(line);
        cur.x() -= 1;
    }
    return cur;
}

std::string Buffer::text() const {
    std::ostringstream ss;

    text(ss);

    return ss.str();
}

void Buffer::text(std::string str) {
    std::istringstream ss{move(str)};

    text(ss);
}

void Buffer::text(std::istream &stream) {
    _lines.clear();

    for (std::string line; getline(stream, line);) {
        _lines.emplace_back(std::move(line));
    }

    colorize(*this);
}

void Buffer::text(std::ostream &stream) const {
    if (_lines.empty()) {
        return;
    }

    stream << std::string{_lines.front()};
    for (size_t i = 1; i < _lines.size(); ++i) {

        stream << "\n" << std::string{_lines.at(i)};
    }
}
