#pragma once

#include "meditfwd.h"
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

struct BufferEvent {
    std::shared_ptr<Buffer> buffer;
    std::shared_ptr<IEnvironment> env;
    enum {
        Open,
        Close,
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

    using BufferSubscriptionT = std::function<void(BufferEvent)>;

    /// TODO: create unsubscribe
    void subscribeToBufferEvents(BufferSubscriptionT f);

    void emitBufferSubscriptionEvent(BufferEvent e);

private:
    std::vector<std::shared_ptr<Buffer>> _buffers;
    std::mutex _fileMutex;

    std::vector<BufferSubscriptionT> _bufferSubscriptions;
};
