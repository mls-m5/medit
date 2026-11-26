#include "file.h"
#include "text/buffer.h"
#include <filesystem>
#include <fstream>
#include <system_error>

File::File(std::filesystem::path path)
    : _path(path) {}

void File::load(Buffer &buffer) {
    std::ifstream file(_path);
    if (file.is_open()) {
        file >> buffer;
        buffer.isChanged(false);
    }
}

bool File::save(const Buffer &buffer) {
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
    return !file.fail();
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
