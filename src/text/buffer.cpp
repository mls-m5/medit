#include "text/buffer.h"
#include "text/colorize.h"
#include "text/cursor.h"
#include <sstream>

Cursor Buffer::begin() {
    return {*this};
}

Cursor Buffer::end() {
    return {*this, _lines.back().size(), _lines.size() - 1};
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
    changed(true);
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
