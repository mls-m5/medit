#include "tcpserver.h"
#include "core/threadname.h"
#include "tcpconnection.h"

using namespace boost::asio;
using namespace boost::asio::ip;
using boost::system::error_code;

struct TcpServer::Impl {
    Impl(int port)
        : acceptor{TcpConnection::context(), tcp::endpoint(tcp::v4(), port)} {
        serverThread = std::thread{[this] {
            setThreadName("tcp server acceptor");
            //            try {
            while (isRunning) {
                tcp::socket socket(TcpConnection::context());

                auto ec = boost::system::error_code{};

                try {
                    acceptor.accept(socket, ec);
                }
                catch (...) {
                    isRunning = false;
                    break;
                }

                if (ec) {
                    if (ec == boost::asio::error::operation_aborted) {
                        isRunning = false;
                        break;
                    }
                }

                if (!isRunning) {
                    break;
                }

                connection = std::make_shared<TcpConnection>(std::move(socket));
                callback(connection);

                connection->_inThread =
                    std::thread{[conn = connection.get(), this] {
                        setThreadName("tcp socket receive thread");
                        conn->inLoop();
                    }};
            }
            //            }
            //            catch (std::exception &e) {
            //                throw e;
            //            }
            isRunning = false;
        }

        };
    }

    std::thread serverThread;

    tcp::acceptor acceptor;

    CallbackT callback;
    bool isRunning = true;
    std::shared_ptr<TcpConnection> connection;
};

TcpServer::TcpServer(int port)
    : _impl{std::make_unique<Impl>(port)} {}

TcpServer::~TcpServer() {
    _impl->isRunning = false;
    _impl->acceptor.close();
    _impl->connection->close();
    _impl->serverThread.join();
}

void TcpServer::accept(CallbackT f) {
    _impl->callback = f;
}
