#pragma once

#include "screen/iconnection.h"
#include <memory>
#include <thread>

class TcpServer {
public:
    using CallbackT = std::function<void(std::shared_ptr<IConnection>)>;

    TcpServer(int port);

    void callback(CallbackT f);

private:
    CallbackT _callback;

    std::thread _serverThread;
};
