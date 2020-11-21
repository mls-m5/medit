#include "text/buffer.h"
//#include "text/colorize.h"
#include "text/cursor.h"
#include <sstream>

Cursor Buffer::cursor(Position pos) {
    forceThread();
    return {*this, pos};
}

Cursor Buffer::begin() {
    forceThread();
    return {*this};
}

Cursor Buffer::end() {
    forceThread();
    return {*this, _lines.back().size(), _lines.size() - 1};
}

FChar Buffer::front() const {
    forceThread();
    if (_lines.empty() || _lines.front().empty()) {
        std::out_of_range("buffer is empty when calling front()");
    }
    return _lines.front().front();
}

FChar Buffer::back() const {
    forceThread();
    if (_lines.empty() || _lines.front().empty()) {
        std::out_of_range("buffer is empty when calling back()");
    }
    return _lines.back().back();
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
    forceThread();
    changed(true);
    _lines.clear();

    for (std::string line; getline(stream, line);) {
        _lines.emplace_back(std::move(line));
    }
}

void Buffer::text(std::ostream &stream) const {
    forceThread();
    if (_lines.empty()) {
        return;
    }

    stream << std::string{_lines.front()};
    for (size_t i = 1; i < _lines.size(); ++i) {
        stream << "\n" << std::string{_lines.at(i)};
    }
}
