#include "coreenvironment.h"
#include "text/buffer.h"

CoreEnvironment::CoreEnvironment() = default;

std::shared_ptr<Buffer> CoreEnvironment::open(std::filesystem::path path) {
    std::scoped_lock g{_fileMutex};
    for (auto &buffer : _buffers) {
        if (buffer->path() == path) {
            return buffer;
        }
    }

    _buffers.push_back(Buffer::open(path));
    return _buffers.back();
}

std::shared_ptr<Buffer> CoreEnvironment::create() {
    _buffers.push_back(std::make_shared<Buffer>());
    return _buffers.back();
}

CoreEnvironment &CoreEnvironment::instance() {
    static CoreEnvironment core;
    return core;
}
