#include "buffer.h"
#include "cursorops.h"
#include "cursorrange.h"
#include "cursorrangeops.h"

std::unique_ptr<Buffer> Buffer::open(std::filesystem::__cxx11::path path) {
    auto buffer = std::make_unique<Buffer>();

    auto file = std::make_unique<File>(path);
    if (std::filesystem::exists(file->path())) {
        file->load(*buffer);
    }
    buffer->_file = std::move(file);
    buffer->_history.clear();
    return buffer;
}

void Buffer::save() {
    _tv();
    if (_file) {
        _file->save(*this);
        _raw.isChanged(false);
    }
}

void Buffer::load() {
    _tv();
    if (_file) {
        _file->load(*this);
        _raw.isChanged(false);
    }
}

std::string Buffer::text() const {
    std::ostringstream ss;

    text(ss);

    return ss.str();
}

void Buffer::pushBack(FString string) {
    _tv();
    string.insert(0, '\n');
    insert(end(), std::move(string));
}

void Buffer::text(std::istream &in) {
    auto ss = std::ostringstream{};

    ss << in.rdbuf();

    auto fstr = FString{ss.str()};

    replace(all(*this), std::move(fstr));
}

void Buffer::format(const CursorRange &range, FormatType f) {
    _tv();
    _raw.format(range, f);
}
