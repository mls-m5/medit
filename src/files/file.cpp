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

File::File(std::filesystem::path path)
    : _path(path) {}

void File::load(Buffer &buffer) {
    if (auto bytes = readDisk()) {
        buffer.replaceText(*bytes);
        buffer.isChanged(false);
        baseline(*bytes);
    }
}

bool File::save(const Buffer &buffer) {
    if (std::filesystem::is_directory(_path)) {
        return false;
    }
    const auto bytes = fileBytes(buffer);
    {
        auto file = std::ofstream{_path, std::ios::binary};
        file << bytes;
        if (!file) {
            return false;
        }
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
    return diskState(readDiskContent(_path));
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
    return readDiskContent(_path).bytes;
}

DiskContent readDiskContent(const std::filesystem::path &path) {
    auto content = DiskContent{};
    auto error = std::error_code{};
    content.exists = std::filesystem::exists(path, error);
    if (!content.exists || std::filesystem::is_directory(path, error)) {
        return content;
    }
    auto file = std::ifstream{path, std::ios::binary};
    if (!file.is_open()) {
        return content;
    }
    auto bytes = std::string{std::istreambuf_iterator<char>{file},
                             std::istreambuf_iterator<char>{}};
    if (!file.bad()) {
        content.bytes = std::move(bytes);
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
