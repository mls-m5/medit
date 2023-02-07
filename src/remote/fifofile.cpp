#include "fifofile.h"

const std::filesystem::path FifoFile::clientInPath =
    std::filesystem::temp_directory_path() / "medit-fifo-client-in";
const std::filesystem::path FifoFile::clientOutPath =
    std::filesystem::temp_directory_path() / "medit-fifo-client-out";

FifoFile::FifoFile(std::filesystem::path path)
    : _path{path} {

    if (std::filesystem::exists(path)) {
        if (std::filesystem::is_fifo(path)) {
            std::filesystem::remove(_path);
        }
    }

    if (std::system(("mkfifo \"" + path.string() + "\"").c_str())) {
        _path.clear();
        throw std::runtime_error{"could not create fifo file " + path.string()};
    }
}

FifoFile::~FifoFile() {
    if (!_path.empty()) {
        std::filesystem::remove(_path);
    }
}
