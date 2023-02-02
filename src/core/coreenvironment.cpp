#include "coreenvironment.h"
#include "text/buffer.h"

CoreEnvironment::CoreEnvironment() = default;

std::shared_ptr<Buffer> CoreEnvironment::open(
    std::filesystem::path path, std::shared_ptr<IEnvironment> env) {
    _tv();
    std::scoped_lock g{_fileMutex};
    for (auto &buffer : _buffers) {
        if (buffer->path() == path) {
            emitBufferSubscriptionEvent({buffer, env, BufferEvent::Open});
            return buffer;
        }
    }

    _buffers.push_back(Buffer::open(path));
    emitBufferSubscriptionEvent({_buffers.back(), env, BufferEvent::Open});
    return _buffers.back();
}

std::shared_ptr<Buffer> CoreEnvironment::create(
    std::shared_ptr<IEnvironment> env) {
    _tv();
    _buffers.push_back(std::make_shared<Buffer>());
    emitBufferSubscriptionEvent({_buffers.back(), env, BufferEvent::Open});
    return _buffers.back();
}

CoreEnvironment &CoreEnvironment::instance() {
    static CoreEnvironment core;
    return core;
}

void CoreEnvironment::subscribeToBufferEvents(BufferSubscriptionT f) {
    _tv();
    _bufferSubscriptions.push_back(f);
}

void CoreEnvironment::emitBufferSubscriptionEvent(BufferEvent e) {
    _tv();
    for (auto &subscriber : _bufferSubscriptions) {
        subscriber(e);
    }
}

void CoreEnvironment::publishDiagnostics(
    std::filesystem::path file,
    std::string source,
    std::vector<Diagnostics::Diagnostic> data) {
    _tv();
    for (auto &buffer : _buffers) {
        if (buffer->path() == file) {
            buffer->diagnostics().publish(source, std::move(data));
            return;
        }
    }
}
