#pragma once

#include "screen/iconnection.h"
#include <memory>
#include <thread>

class TcpServer {
public:
    using CallbackT = std::function<void(std::shared_ptr<IConnection>)>;

    TcpServer(const TcpServer &) = delete;
    TcpServer(TcpServer &&) = delete;
    TcpServer &operator=(const TcpServer &) = delete;
    TcpServer &operator=(TcpServer &&) = delete;

    TcpServer(int port);
    ~TcpServer();

    /// Callback function for when a user is connected
    void accept(CallbackT f);

private:
    struct Impl;

    std::unique_ptr<Impl> _impl;
};
