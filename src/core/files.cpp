#include "files.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "core/itimer.h"
#include "files/file.h"
#include "linux/inotify.h"
#include "text/buffer.h"
#include <iostream>

Files::Files(CoreEnvironment &core)
    : _core{core}
    , _dirNotifications{
          createDirectoryNotifications(_core.context().jobQueue())} {
    _dirNotifications->subscribe(
        [this](auto a, auto b, auto c) { fileChangeCallback(a, b, c); }, this);
}

Files::~Files() = default;

std::shared_ptr<Buffer> Files::open(std::filesystem::path path) {
    _tv();

    if (auto buffer = find(path)) {
        emitBufferSubscriptionEvent({buffer, BufferEvent::Open});
        return buffer;
    }

    // TODO: Maybe subscribe to buffer changes here and publish them or just
    // use theme to update highlighting

    _buffers.push_back(Buffer::open(path));
    emitBufferSubscriptionEvent({_buffers.back(), BufferEvent::Open});
    return _buffers.back();
}

std::shared_ptr<Buffer> Files::find(std::filesystem::path path) {
    _tv();
    path = std::filesystem::absolute(path);
    for (auto &buffer : _buffers) {
        if (path == buffer->path()) {
            return buffer;
        }
    }
    return nullptr;
}

void Files::fileChangeCallback(DirectoryNotifications::EventType type,
                               std::filesystem::path path,
                               std::filesystem::file_time_type time) {

    // TODO: Handle deleted files later
    if (type != DirectoryNotifications::Changed &&
        type != DirectoryNotifications::Added) {
        return;
    }

    _core.context().guiQueue().addTask([this, type, path, time] {
        _tv();
        if (auto f = find(path)) {
            // TODO: Double check so that there is no unsaved changes in the
            // document
            f->load();
        }
    });
}

std::shared_ptr<Buffer> Files::create() {
    _tv();
    _buffers.push_back(std::make_shared<Buffer>());
    emitBufferSubscriptionEvent({_buffers.back(), BufferEvent::Open});
    return _buffers.back();
}

void Files::save(Buffer &buffer, std::filesystem::path path) {
    _tv();

    buffer.assignFile(std::make_unique<File>(std::filesystem::absolute(path)));
    buffer.save();

    _buffers.push_back(buffer.shared_from_this());
    emitBufferSubscriptionEvent({_buffers.back(), BufferEvent::Open});
}

void Files::unsubscribeToBufferEvents(void *reference) {
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

void Files::emitBufferSubscriptionEvent(BufferEvent e) {
    _tv();
    for (auto &subscriber : _bufferSubscriptions) {
        subscriber.f(e);
    }
}

void Files::publishDiagnostics(std::filesystem::path file,
                               std::string source,
                               std::vector<Diagnostics::Diagnostic> data) {
    _tv();
    for (auto &buffer : _buffers) {
        if (buffer->path() == file) {
            buffer->diagnostics().publish(source, std::move(data));
            buffer->emitChangeSignal();
            return;
        }
    }
}

void Files::subscribeToBufferEvents(BufferSubscriptionCallbackT f,
                                    Buffer *buffer,
                                    void *ref) {
    _tv();
    _bufferSubscriptions.push_back({f, buffer, ref});
}

void Files::updateHighlighting() {
    auto updateFunction = [this]() {
        for (auto &buffer : _buffers) {
            if (!buffer->isColorsOld()) {
                continue;
            }
            for (auto &highlight : _core.plugins().get<IHighlight>()) {
                if (highlight->shouldEnable(buffer->path())) {
                    highlight->highlight(*buffer);

                    buffer->isColorsOld(false);
                    break;
                }
            }
            for (auto &annotation : _core.plugins().get<IAnnotation>()) {
                if (annotation->shouldEnable(buffer->path())) {
                    annotation->annotate(*buffer);
                    break;
                }
            }
        }
    };

#ifdef __EMSCRIPTEN__
    return; // Todo: find reason for crash
#endif      //__EMSCRIPTEN__

    auto &timer = _core.context().timer();

    if (_updateTimeHandle) {
        timer.cancel(_updateTimeHandle);
        _updateTimeHandle = 0;
    }

    using namespace std::literals;

    _updateTimeHandle = timer.setTimeout(1s, [this, updateFunction] {
        auto &queue = _core.context().guiQueue();

        queue.addTask([this, updateFunction] { updateFunction(); });
    });
}
