#include "file.h"
#include "text/buffer.h"
#include <filesystem>
#include <fstream>
#include <system_error>
#if defined(__linux__) && !defined(__EMSCRIPTEN__)
#include <cerrno>
#include <sys/stat.h>
#endif

File::File(std::filesystem::path path)
    : _path(path) {
    _diskStateKnown = readDiskState(_diskState);
}

bool File::readDiskState(std::optional<DiskState> &state) const {
#if defined(__linux__) && !defined(__EMSCRIPTEN__)
    struct stat metadata;
    if (::stat(_path.c_str(), &metadata) != 0) {
        if (errno == ENOENT || errno == ENOTDIR) {
            state.reset();
            return true;
        }
        return false;
    }
    auto type = std::filesystem::file_type::unknown;
    if (S_ISREG(metadata.st_mode))
        type = std::filesystem::file_type::regular;
    else if (S_ISDIR(metadata.st_mode))
        type = std::filesystem::file_type::directory;
    state = DiskState{.size = static_cast<std::uintmax_t>(metadata.st_size),
                      .device = static_cast<std::uintmax_t>(metadata.st_dev),
                      .inode = static_cast<std::uintmax_t>(metadata.st_ino),
                      .modifiedSeconds = metadata.st_mtim.tv_sec,
                      .modifiedNanoseconds = metadata.st_mtim.tv_nsec,
                      .changedSeconds = metadata.st_ctim.tv_sec,
                      .changedNanoseconds = metadata.st_ctim.tv_nsec,
                      .type = type};
    return true;
#else
    std::error_code error;
    const auto status = std::filesystem::status(_path, error);
    if (error == std::errc::no_such_file_or_directory) {
        state.reset();
        return true;
    }
    if (error)
        return false;
    const auto size = std::filesystem::file_size(_path, error);
    if (error)
        return false;
    const auto modified = std::filesystem::last_write_time(_path, error);
    if (error)
        return false;
    state = DiskState{.size = size,
                      .modifiedSeconds = modified.time_since_epoch().count(),
                      .type = status.type()};
    return true;
#endif
}

bool File::hasExternalChanges() const {
    std::optional<DiskState> state;
    return !_diskStateKnown || !readDiskState(state) || state != _diskState;
}

void File::load(Buffer &buffer) {
    std::optional<DiskState> before, after;
    const bool beforeKnown = readDiskState(before);
    std::ifstream file(_path);
    if (file.is_open()) {
        file >> buffer;
        if (!file.bad()) {
            buffer.isChanged(false);
            _diskStateKnown =
                beforeKnown && readDiskState(after) && before == after;
            _diskState = after;
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
    _diskStateKnown = readDiskState(_diskState);
    return true;
}

std::string File::representation() const {
    return _path.string();
}

std::filesystem::path File::path() const {
    return _path;
}

void File::rename(std::filesystem::path to, bool externallyChanged) {
    // A rename can change ctime. Refresh a clean file's baseline, but never
    // turn an already conflicting disk version into an accepted one.
    _path = to;
    _diskStateKnown = !externallyChanged && readDiskState(_diskState);
}
