#pragma once

#include "core/threadvalidation.h"
#include "files/directorynotifications.h"
#include "files/diskstate.h"
#include "files/filewatcher.h"
#include "meditfwd.h"
#include "text/diagnostics.h"
#include <filesystem>
#include <functional>
#include <map>
#include <memory>

struct BufferEvent {
    std::shared_ptr<Buffer> buffer;
    std::filesystem::path path; // This is separate for when the path is changed
    enum {
        Open,
        Close,
    } type = Open;
};

/// Managing opening files to buffers
class Files {
public:
    Files(CoreEnvironment &core);

    Files(const Files &) = delete;
    Files(Files &&) = delete;
    Files &operator=(const Files &) = delete;
    Files &operator=(Files &&) = delete;
    ~Files();

    std::shared_ptr<Buffer> open(std::filesystem::path);
    std::shared_ptr<Buffer> create();

    /// Use this when saving a buffer that does not have a file
    /// @return true if saved successfull
    bool save(Buffer &, std::filesystem::path);

    using BufferSubscriptionCallbackT = std::function<void(BufferEvent)>;

    //! @param buffer which buffer to subscribe to, null if all buffers
    //! @param reference a reference to be used when unsubscibing, can be null
    void subscribeToBufferEvents(BufferSubscriptionCallbackT f,
                                 Buffer *buffer,
                                 void *reference);

    void unsubscribeToBufferEvents(void *reference);

    void emitBufferSubscriptionEvent(BufferEvent e);

    void updateHighlighting();

    void publishDiagnostics(std::filesystem::path file,
                            std::string source,
                            std::vector<Diagnostics::Diagnostic>);

    // Return a buffer only if it is loaded
    std::shared_ptr<Buffer> find(std::filesystem::path path);

    DirectoryNotifications &directoryNotifications() {
        return *_dirNotifications;
    }

    bool rename(std::filesystem::path from, std::filesystem::path to);

    /// Forgets the buffer and emits BufferEvent::Close for it, so that views
    /// stop showing it. Used for a file deleted on disk without unsaved
    /// changes.
    void close(std::shared_ptr<Buffer> buffer);

    /// Stops watching open files for changes on disk. Called before the job
    /// queues stop, since the watcher posts to them.
    void stopWatching();

private:
    CoreEnvironment &_core;

    std::vector<std::shared_ptr<Buffer>> _buffers;
    std::unique_ptr<DirectoryNotifications> _dirNotifications;

    /// Watching open files for changes on disk (see
    /// docs/file-reload-design.md): a hint from the watcher is debounced,
    /// the file is read on the job thread, and the result is applied on the
    /// GUI thread unless the buffer was loaded or saved meanwhile.
    void watchFile(const std::filesystem::path &file);
    void unwatchFile(const std::filesystem::path &file);
    void fileChanged(const std::filesystem::path &path);
    void startCheck(const std::filesystem::path &path);
    void finishCheck(const std::filesystem::path &path,
                     size_t baselineVersion,
                     const DiskContent &content);

    std::unique_ptr<FileWatcher> _watcher;
    std::map<std::filesystem::path, size_t> _pendingChecks; // Timer ids
    /// Expires with this object; queued tasks check it before using `this`.
    std::shared_ptr<int> _alive = std::make_shared<int>();

    struct BufferSubscription {
        BufferSubscriptionCallbackT f;
        Buffer *buffer = nullptr;
        void *ref = nullptr;
    };

    std::vector<BufferSubscription> _bufferSubscriptions;

    size_t _updateTimeHandle = 0;

    ThreadValidation _tv{"core thread (gui thread)"};
};
