#include "text/rawbuffer.h"
#include "text/cursor.h"
#include <sstream>

Cursor RawBuffer::cursor(Position pos) {
    forceThread();
    return {*this, pos};
}

Cursor RawBuffer::begin() {
    forceThread();
    return {*this};
}

Cursor RawBuffer::end() {
    forceThread();
    return {*this, _lines.back().size(), _lines.size() - 1};
}

FChar RawBuffer::front() const {
    forceThread();
    if (_lines.empty() || _lines.front().empty()) {
        std::out_of_range("RawBuffer is empty when calling front()");
    }
    return _lines.front().front();
}

FChar RawBuffer::back() const {
    forceThread();
    if (_lines.empty() || _lines.front().empty()) {
        std::out_of_range("RawBuffer is empty when calling back()");
    }
    return _lines.back().back();
}

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
    forceThread();
    isChanged(true);
    _lines.clear();

    for (std::string line; getline(stream, line);) {
        _lines.emplace_back(std::move(line));
    }
}

void RawBuffer::text(std::ostream &stream) const {
    forceThread();
    if (_lines.empty()) {
        return;
    }

    stream << std::string{_lines.front()};
    for (size_t i = 1; i < _lines.size(); ++i) {
        stream << "\n" << std::string{_lines.at(i)};
    }
}

FString RawBuffer::ftext() const {
    size_t size = _lines.size() - 1;

    for (auto &line : _lines) {
        size += line.size();
    }

    auto ret = FString{};
    ret.resize(size, FChar{"\n"});

    size_t i = 0;
    for (auto &line : _lines) {
        for (auto c : line) {
            ret.at(i) = c;
            ++i;
        }
        ++i;
    }

    return ret;
}

void RawBuffer::forceThread() const {
    if (std::this_thread::get_id() != _threadId) {
        auto intId = [](auto id) {
            auto ss = std::ostringstream{};
            ss << id;
            return ss.str();
        };
        throw std::runtime_error{
            "buffer called from another thread than the one started "
            "from: " +
            intId(std::this_thread::get_id()) + " vs started from " +
            intId(_threadId)};
    }
}
