#pragma once

#include "core/context.h"
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
    //    std::shared_ptr<IEnvironment> env;
    enum {
        Open,
        Close,
        Redraw, // For example new diagnostics is published or buffer changed
    } type = Open;
};

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

    Context &context() {
        return *_context;
    }

    void context(Context *c) {
        _context = c;
    }

private:
    std::vector<std::shared_ptr<Buffer>> _buffers;
    std::mutex _fileMutex;

    ThreadValidation _tv{"core thread (gui thread)"};

    struct BufferSubscription {
        BufferSubscriptionCallbackT f;
        Buffer *buffer = nullptr;
        void *ref = nullptr;
    };

    std::vector<BufferSubscription> _bufferSubscriptions;

    Context *_context =
        nullptr; // TODO: Handle lifetime of CoreEnvironment better
};
