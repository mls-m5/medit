#pragma once

#include "screen/iconnection.h"
#include <memory>
#include <thread>

class TcpServer {
public:
    using CallbackT = std::function<void(std::shared_ptr<IConnection>)>;

    TcpServer(int port);
    ~TcpServer();

    void callback(CallbackT f);

private:
    struct Impl;

    std::unique_ptr<Impl> _impl;
};
