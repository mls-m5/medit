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

void File::save(const Buffer &buffer) {
    if (std::filesystem::is_directory(_path)) {
        return;
    }
    if (buffer.lines().back().empty()) {
        std::ofstream{_path} << buffer;
    }
    else {
        std::ofstream{_path} << buffer << "\n";
    }
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
