#include "file.h"
#include "text/buffer.h"
#include <fstream>

File::File(filesystem::path path) : _path(path) {}

void File::load(Buffer &buffer) {
    std::ifstream file(_path);
    if (file.is_open()) {
        buffer.text(file);
        buffer.isChanged(false);
    }
}

void File::save(const Buffer &buffer) {
    auto file = std::ofstream{_path};
    buffer.text(file);
    file << "\n";
}

std::string File::representation() const {
    return _path.string();
}

filesystem::path File::path() const {
    return _path;
}
