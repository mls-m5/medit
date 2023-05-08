#pragma once

#include "core/context.h"
#include "core/plugins.h"
#include "meditfwd.h"
#include "text/diagnostics.h"
#include "threadvalidation.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

struct BufferEvent {
    std::shared_ptr<Buffer> buffer;
    enum {
        Open,
        Close,
        //        Redraw, // For example new diagnostics is published or buffer
        //        changed
    } type = Open;
};

// TODO: Break out file handling functionality
/// Environment shared with all user of the program/server
class CoreEnvironment {
public:
    CoreEnvironment();

    CoreEnvironment(const CoreEnvironment &) = delete;
    CoreEnvironment(CoreEnvironment &&) = delete;
    CoreEnvironment &operator=(const CoreEnvironment &) = delete;
    CoreEnvironment &operator=(CoreEnvironment &&) = delete;

    std::shared_ptr<Buffer> open(std::filesystem::path,
                                 std::shared_ptr<IEnvironment> env);
    std::shared_ptr<Buffer> create(std::shared_ptr<IEnvironment> env);

    static CoreEnvironment &instance();

    using BufferSubscriptionCallbackT = std::function<void(BufferEvent)>;

    //! @param buffer which buffer to subscribe to, null if all buffers
    //! @param reference a reference to be used when unsubscibing, can be null
    void subscribeToBufferEvents(BufferSubscriptionCallbackT f,
                                 Buffer *buffer,
                                 void *reference);

    void unsubscribeToBufferEvents(void *reference);

    void emitBufferSubscriptionEvent(BufferEvent e);

    void publishDiagnostics(std::filesystem::path file,
                            std::string source,
                            std::vector<Diagnostics::Diagnostic>);

    ThreadContext &context() {
        return *_context;
    }

    void context(ThreadContext *c) {
        _context = c;
    }

    Plugins &plugins() {
        return _plugins;
    }

    // Return a buffer only if it is loaded
    std::shared_ptr<Buffer> find(std::filesystem::path path);

private:
    std::vector<std::shared_ptr<Buffer>> _buffers;
    std::mutex _fileMutex;
    Plugins _plugins;

    ThreadValidation _tv{"core thread (gui thread)"};

    struct BufferSubscription {
        BufferSubscriptionCallbackT f;
        Buffer *buffer = nullptr;
        void *ref = nullptr;
    };

    std::vector<BufferSubscription> _bufferSubscriptions;

    ThreadContext *_context =
        nullptr; // TODO: Handle lifetime of CoreEnvironment better
};
