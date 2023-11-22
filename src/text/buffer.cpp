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
        buffer->_history.clear();
    }
    return buffer;
}

bool Buffer::save() {
    _tv();
    if (_file) {
        if (_file->save(*this)) {
            _raw.isChanged(false);
            return true;
        }
        return false;
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
        _file->load(*this);
        _raw.isChanged(false);
        emitChangeSignal();
    }
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
    auto cur = _raw.apply(std::move(edit));
    emitChangeSignal();
    return cur;
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
