#include "file.h"
#include "text/buffer.h"
#include <filesystem>
#include <fstream>
#include <system_error>

File::File(std::filesystem::path path)
    : _path(path) {
    _diskTimeKnown = readDiskTime(_diskTime);
}

bool File::readDiskTime(
    std::optional<std::filesystem::file_time_type> &time) const {
    std::error_code error;
    const auto value = std::filesystem::last_write_time(_path, error);
    if (error == std::errc::no_such_file_or_directory) {
        time.reset();
        return true;
    }
    if (error)
        return false;
    time = value;
    return true;
}

bool File::hasExternalChanges() const {
    std::optional<std::filesystem::file_time_type> time;
    return !_diskTimeKnown || !readDiskTime(time) || time != _diskTime;
}

void File::load(Buffer &buffer) {
    std::optional<std::filesystem::file_time_type> before, after;
    const bool beforeKnown = readDiskTime(before);
    std::ifstream file(_path);
    if (file.is_open()) {
        file >> buffer;
        if (!file.bad()) {
            buffer.isChanged(false);
            _diskTimeKnown =
                beforeKnown && readDiskTime(after) && before == after;
            _diskTime = after;
        }
    }
}

bool File::save(const Buffer &buffer, bool overwriteExternal) {
    if (!overwriteExternal && hasExternalChanges())
        return false;
    if (std::filesystem::is_directory(_path)) {
        return false;
    }
    std::ofstream file{_path};
    if (!file.is_open())
        return false;
    file << buffer;
    if (!buffer.lines().back().empty())
        file << "\n";
    file.close();
    if (file.fail())
        return false;
    _diskTimeKnown = readDiskTime(_diskTime);
    return true;
}

std::string File::representation() const {
    return _path.string();
}

std::filesystem::path File::path() const {
    return _path;
}

void File::rename(std::filesystem::path to) {
    _path = to;
}
