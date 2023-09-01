#pragma once

#include "screen/iconnection.h"
#include <filesystem>
#include <fstream>
#include <thread>

//! Could be used as a IConnection, but could also be used to connect to another
//! connection
class FifoConnection : public IConnection {
public:
    FifoConnection(std::filesystem::path in, std::filesystem::path out);

    ~FifoConnection();

    void write(std::string_view str) override;
    void subscribe(IConnection::CallbackT) override;
    void unsubscribe() override;
    void close() override;

    /// Stop execution from this thread until pipes is closed
    void waitForClose() override;

private:
    void inLoop();

    void handleInput(std::string_view data);

    std::function<void(std::string_view)> _callback;

    std::ofstream _out;
    std::ifstream _in;
    std::thread _inThread;
    std::string _buffer;
};
