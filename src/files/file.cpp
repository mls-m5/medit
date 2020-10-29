
#include "file.h"
#include "text/buffer.h"
#include <fstream>

File::File(filesystem::path path) : _path(path) {}

void File::load(Buffer &buffer) {
    std::ifstream file(_path);
    buffer.text(file);
}

void File::save(const Buffer &buffer) {
    auto file = std::ofstream{_path};
    buffer.text(file);
}
