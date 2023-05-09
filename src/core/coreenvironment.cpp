#include "coreenvironment.h"
#include "files/file.h"
#include "text/buffer.h"
#include <filesystem>

CoreEnvironment::CoreEnvironment() {}

std::shared_ptr<Buffer> CoreEnvironment::open(
    std::filesystem::path path, std::shared_ptr<IEnvironment> env) {
    _tv();
    std::scoped_lock g{_fileMutex};

    if (auto buffer = find(path)) {
        emitBufferSubscriptionEvent({buffer, BufferEvent::Open});
        return buffer;
    }

    _buffers.push_back(Buffer::open(path));
    emitBufferSubscriptionEvent({_buffers.back(), BufferEvent::Open});
    return _buffers.back();
}

std::shared_ptr<Buffer> CoreEnvironment::create(
    std::shared_ptr<IEnvironment> env) {
    _tv();
    _buffers.push_back(std::make_shared<Buffer>());
    emitBufferSubscriptionEvent({_buffers.back(), BufferEvent::Open});
    return _buffers.back();
}

void CoreEnvironment::save(Buffer &buffer, std::filesystem::path path) {
    _tv();
    std::scoped_lock g{_fileMutex};

    buffer.assignFile(std::make_unique<File>(std::filesystem::absolute(path)));
    buffer.save();

    _buffers.push_back(buffer.shared_from_this());
    emitBufferSubscriptionEvent({_buffers.back(), BufferEvent::Open});
}

CoreEnvironment &CoreEnvironment::instance() {
    static CoreEnvironment core;
    return core;
}

void CoreEnvironment::subscribeToBufferEvents(BufferSubscriptionCallbackT f,
                                              Buffer *buffer,
                                              void *ref) {
    _tv();
    _bufferSubscriptions.push_back({f, buffer, ref});
}

void CoreEnvironment::unsubscribeToBufferEvents(void *reference) {
    _tv();

    if (!reference) {
        return;
    }

    auto it =
        std::remove_if(_bufferSubscriptions.begin(),
                       _bufferSubscriptions.end(),
                       [reference](auto &s) { return s.ref == reference; });

    _bufferSubscriptions.erase(it, _bufferSubscriptions.end());
}

void CoreEnvironment::emitBufferSubscriptionEvent(BufferEvent e) {
    _tv();
    for (auto &subscriber : _bufferSubscriptions) {
        subscriber.f(e);
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
            //            emitBufferSubscriptionEvent({buffer,
            //            BufferEvent::Redraw});
            buffer->emitChangeSignal();
            //            _context->redrawScreen();
            return;
        }
    }
}

std::shared_ptr<Buffer> CoreEnvironment::find(std::filesystem::path path) {
    _tv();
    path = std::filesystem::absolute(path);
    for (auto &buffer : _buffers) {
        if (path == buffer->path()) {
            return buffer;
        }
    }
    return nullptr;
}
