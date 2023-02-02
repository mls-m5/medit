#include "file.h"
#include "text/buffer.h"
#include <fstream>

File::File(filesystem::path path)
    : _path(path) {}

void File::load(RawBuffer &buffer) {
    std::ifstream file(_path);
    if (file.is_open()) {
        file >> buffer;
        buffer.isChanged(false);
    }
}

void File::save(const RawBuffer &buffer) {
    std::ofstream{_path} << buffer << "\n";
}

std::string File::representation() const {
    return _path.string();
}

filesystem::path File::path() const {
    return _path;
}
