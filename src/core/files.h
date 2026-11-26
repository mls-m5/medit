#pragma once

#include "core/threadvalidation.h"
#include "files/directorynotifications.h"
#include "meditfwd.h"
#include "text/diagnostics.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <unordered_map>

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

    /// Save a modified disk-backed buffer now (e.g. before leaving its view).
    /// Conflicts and failures are reported once per edit; no UI interaction.
    void saveIfChanged(Buffer &);
    void saveChangedFiles();

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

    // Includes buffers no longer shown in an editor, which can still hold
    // unsaved content. The GUI thread owns this list.
    std::vector<std::shared_ptr<Buffer>> buffers() const {
        _tv();
        return _buffers;
    }

    DirectoryNotifications &directoryNotifications() {
        return *_dirNotifications;
    }

    bool rename(std::filesystem::path from, std::filesystem::path to);

private:
    CoreEnvironment &_core;

    std::vector<std::shared_ptr<Buffer>> _buffers;
    std::unique_ptr<DirectoryNotifications> _dirNotifications;

    // Function that is called by the DirectoryNotifications
    void trackAutosave(const std::shared_ptr<Buffer> &);
    void bufferChanged(const std::shared_ptr<Buffer> &);

    struct AutosaveState {
        size_t timer = 0;
        size_t generation = 0;
        size_t editRevision = 0;
        bool errorReported = false;
    };
    std::unordered_map<Buffer *, AutosaveState> _autosave;
    std::shared_ptr<int> _lifetime = std::make_shared<int>(0);

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
