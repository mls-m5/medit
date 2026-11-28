#include "file.h"
#include "text/buffer.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <string_view>
#include <system_error>

std::string fileBytes(const Buffer &buffer) {
    auto bytes = buffer.text();
    if (!buffer.lines().back().empty()) {
        bytes += '\n';
    }
    return bytes;
}

File::File(std::filesystem::path path, IFileSystem &fileSystem)
    : _path(path)
    , _fileSystem{&fileSystem} {}

void File::load(Buffer &buffer) {
    if (auto bytes = readDisk()) {
        buffer.replaceText(*bytes);
        buffer.isChanged(false);
        baseline(*bytes);
    }
}

bool File::save(const Buffer &buffer) {
    const auto bytes = fileBytes(buffer);
    if (!_fileSystem->write(_path, bytes)) {
        return false;
    }
    baseline(bytes);
    return true;
}

std::string File::representation() const {
    return _path.string();
}

std::filesystem::path File::path() const {
    return _path;
}

DiskState File::diskState() const {
    if (!_baseline) {
        return DiskState::InSync;
    }
    return diskState(diskContent());
}

DiskState File::diskState(const DiskContent &content) const {
    if (!_baseline) {
        return DiskState::InSync;
    }
    if (!content.exists) {
        return DiskState::DeletedOnDisk;
    }
    if (!content.bytes) {
        // Exists but cannot be read now (e.g. being replaced): no evidence of
        // a change.
        return DiskState::InSync;
    }
    const auto &bytes = *content.bytes;
    if (std::hash<std::string_view>{}(bytes) != _baseline->hash ||
        (_baseline->text && *_baseline->text != bytes)) {
        return DiskState::ChangedOnDisk;
    }
    return DiskState::InSync;
}

void File::loaded(const std::string &bytes) {
    baseline(bytes);
}

const std::string *File::baseText() const {
    if (_baseline && _baseline->text) {
        return &*_baseline->text;
    }
    return nullptr;
}

std::optional<std::string> File::readDisk() const {
    return _fileSystem->read(_path);
}

DiskContent File::diskContent() const {
    return readDiskContent(*_fileSystem, _path);
}

DiskContent readDiskContent(IFileSystem &fileSystem,
                            const std::filesystem::path &path) {
    auto content = DiskContent{};
    content.exists = fileSystem.stat(path).exists;
    if (content.exists) {
        content.bytes = fileSystem.read(path);
    }
    return content;
}

void File::baseline(const std::string &bytes) {
    ++_baselineVersion;
    _baseline = Baseline{std::hash<std::string_view>{}(bytes), std::nullopt};
    if (bytes.size() <= maxBaseSize) {
        _baseline->text = bytes;
    }
}

void File::rename(std::filesystem::path to) {
    _path = to;
}
