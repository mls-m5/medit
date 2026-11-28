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
    auto error = std::error_code{};
    if (!std::filesystem::exists(_path, error)) {
        return DiskState::DeletedOnDisk;
    }
    const auto bytes = readDisk();
    if (!bytes) {
        // Exists but cannot be read now (e.g. being replaced): no evidence of
        // a change.
        return DiskState::InSync;
    }
    if (std::hash<std::string_view>{}(*bytes) != _baseline->hash ||
        (_baseline->text && *_baseline->text != *bytes)) {
        return DiskState::ChangedOnDisk;
    }
    return DiskState::InSync;
}

const std::string *File::baseText() const {
    if (_baseline && _baseline->text) {
        return &*_baseline->text;
    }
    return nullptr;
}

std::optional<std::string> File::readDisk() const {
    auto file = std::ifstream{_path, std::ios::binary};
    if (!file.is_open()) {
        return std::nullopt;
    }
    auto bytes = std::string{std::istreambuf_iterator<char>{file},
                             std::istreambuf_iterator<char>{}};
    if (file.bad()) {
        return std::nullopt;
    }
    return bytes;
}

void File::baseline(const std::string &bytes) {
    _baseline = Baseline{std::hash<std::string_view>{}(bytes), std::nullopt};
    if (bytes.size() <= maxBaseSize) {
        _baseline->text = bytes;
    }
}

void File::rename(std::filesystem::path to) {
    _path = to;
}
