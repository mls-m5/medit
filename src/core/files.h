#pragma once

#include "core/threadvalidation.h"
#include "linux/inotify.h"
#include "meditfwd.h"
#include "text/diagnostics.h"
#include <filesystem>
#include <functional>
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
    ~Files();

    std::shared_ptr<Buffer> open(std::filesystem::path);
    std::shared_ptr<Buffer> create();

    /// Use this when saving a buffer that does not have a file
    void save(Buffer &, std::filesystem::path);

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

private:
    CoreEnvironment &_core;

    std::vector<std::shared_ptr<Buffer>> _buffers;
    std::unique_ptr<DirectoryNotifications> _dirNotifications;

    // Function that is called by the DirectoryNotifications
    void fileChangeCallback(DirectoryNotifications::EventType,
                            std::filesystem::path,
                            std::filesystem::file_time_type);

    struct BufferSubscription {
        BufferSubscriptionCallbackT f;
        Buffer *buffer = nullptr;
        void *ref = nullptr;
    };

    std::vector<BufferSubscription> _bufferSubscriptions;

    size_t _updateTimeHandle = 0;

    ThreadValidation _tv{"core thread (gui thread)"};
};
