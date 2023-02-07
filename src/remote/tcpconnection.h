#pragma once

#include "screen/iconnection.h"
#include <boost/asio/ip/tcp.hpp>
#include <filesystem>
#include <fstream>
#include <thread>

//! Could be used as a IConnection, but could also be used to connect to another
//! connection
class TcpConnection : public IConnection,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    using SocketT = boost::asio::ip::tcp::socket;

    //! Connect to remote server as client
    //! @see TcpServer for hosting
    static std::shared_ptr<TcpConnection> connect(std::string address,
                                                  int port);

    TcpConnection(SocketT socket)
        : _socket{std::move(socket)} {}

    ~TcpConnection();

    void write(std::string_view str) override;
    void subscribe(IConnection::CallbackT) override;
    void unsubscribe() override;
    void close() override;

    /// Stop execution from this thread until pipes is closed
    void waitForClose();

private:
    static boost::asio::io_context &context();

    void inLoop();

    void handleInput(std::string_view data);

    std::function<void(std::string_view)> _callback;

    std::thread _inThread;
    std::string _buffer;

    SocketT _socket;

    friend class TcpServer;
};
