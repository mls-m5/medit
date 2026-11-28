#include "files.h"
#include "core/context.h"
#include "core/coreenvironment.h"
#include "core/ijobqueue.h"
#include "core/itimer.h"
#include "files/diskcheck.h"
#include "files/file.h"
#include "files/filewatcher.h"
#include "linux/inotify.h"
#include "text/buffer.h"
#include <filesystem>

Files::Files(CoreEnvironment &core)
    : _core{core}
    , _dirNotifications{
          createDirectoryNotifications(_core.context().jobQueue())} {
    auto *gui = &_core.context().guiQueue();
    _watcher = createFileWatcher(
        [gui, alive = std::weak_ptr{_alive}, this](auto &path) {
            gui->addTask([alive, this, path] {
                if (!alive.expired()) {
                    fileChanged(path);
                }
            });
        });
}

Files::~Files() {
    stopWatching();
}

void Files::stopWatching() {
    if (_watcher) {
        _watcher->stop();
    }
    for (auto &pending : _pendingChecks) {
        _core.context().timer().cancel(pending.second);
    }
    _pendingChecks.clear();
}

void Files::watchFile(const std::filesystem::path &file) {
    _watcher->watch(std::filesystem::absolute(file).parent_path());
}

void Files::unwatchFile(const std::filesystem::path &file) {
    _watcher->unwatch(std::filesystem::absolute(file).parent_path());
}

void Files::fileChanged(const std::filesystem::path &path) {
    _tv();
    if (!find(path)) {
        return;
    }
    // Debounce: writers often touch a file several times in a row.
    using namespace std::chrono_literals;
    auto &timer = _core.context().timer();
    if (auto pending = _pendingChecks.find(path);
        pending != _pendingChecks.end()) {
        timer.cancel(pending->second);
    }
    auto *gui = &_core.context().guiQueue();
    _pendingChecks[path] = timer.setTimeout(
        100ms, [gui, alive = std::weak_ptr{_alive}, this, path] {
            gui->addTask([alive, this, path] {
                if (!alive.expired()) {
                    startCheck(path);
                }
            });
        });
}

void Files::startCheck(const std::filesystem::path &path) {
    _tv();
    _pendingChecks.erase(path);
    auto buffer = find(path);
    if (!buffer || !buffer->file()) {
        return;
    }
    const auto version = buffer->file()->baselineVersion();
    auto *gui = &_core.context().guiQueue();
    _core.context().jobQueue().addTask(
        [gui, alive = std::weak_ptr{_alive}, this, path, version] {
            auto content = readDiskContent(path);
            gui->addTask([alive, this, path, version, content] {
                if (!alive.expired()) {
                    finishCheck(path, version, content);
                }
            });
        });
}

void Files::finishCheck(const std::filesystem::path &path,
                        size_t baselineVersion,
                        const DiskContent &content) {
    _tv();
    auto buffer = find(path);
    if (!buffer || !buffer->file()) {
        return;
    }
    if (buffer->file()->baselineVersion() != baselineVersion) {
        // Loaded or saved while the file was read: check again.
        fileChanged(path);
        return;
    }
    reloadFromDisk(*buffer, content);
}

std::shared_ptr<Buffer> Files::open(std::filesystem::path path) {
    _tv();

    if (auto buffer = find(path)) {
        emitBufferSubscriptionEvent(
            {buffer, buffer->file()->path(), BufferEvent::Open});
        return buffer;
    }

    // TODO: Maybe subscribe to buffer changes here and publish them or just
    // use theme to update highlighting

    _buffers.push_back(Buffer::open(path));
    if (_buffers.back()->file()) {
        watchFile(_buffers.back()->file()->path());
    }
    emitBufferSubscriptionEvent(
        {_buffers.back(), _buffers.back()->file()->path(), BufferEvent::Open});
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

bool Files::rename(std::filesystem::path from, std::filesystem::path to) {
    _tv();

    if (from == to) {
        return true;
    }

    if (find(to)) {
        return true;
    }

    if (std::filesystem::exists(to)) {
        return true;
    }

    auto buffer = find(from);

    if (!buffer) {
        return true;
    }

    auto file = buffer->file();

    if (!file) {
        return true;
    }

    std::error_code ec;

    std::filesystem::rename(from, to, ec);

    if (ec) {
        return true;
    }

    unwatchFile(from);
    file->rename(to);
    watchFile(to);

    emitBufferSubscriptionEvent({
        .buffer = nullptr,
        .path = from,
        .type = BufferEvent::Close,
    });
    emitBufferSubscriptionEvent({
        .buffer = buffer,
        .path = to,
        .type = BufferEvent::Open,
    });

    return false;
}

std::shared_ptr<Buffer> Files::create() {
    _tv();
    _buffers.push_back(std::make_shared<Buffer>());
    emitBufferSubscriptionEvent({_buffers.back(), {}, BufferEvent::Open});
    return _buffers.back();
}

bool Files::save(Buffer &buffer, std::filesystem::path path) {
    _tv();

    buffer.assignFile(std::make_unique<File>(std::filesystem::absolute(path)));
    if (!buffer.save()) {
        return false;
    }

    _buffers.push_back(buffer.shared_from_this());
    watchFile(buffer.file()->path());
    emitBufferSubscriptionEvent(
        {_buffers.back(), _buffers.back()->file()->path(), BufferEvent::Open});

    return true;
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
                if (highlight->highlight(*buffer)) {

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
