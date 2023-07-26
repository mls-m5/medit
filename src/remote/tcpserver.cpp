#include "tcpserver.h"
#include "core/threadname.h"
#include "tcpconnection.h"

using namespace boost::asio;
using namespace boost::asio::ip;
using boost::system::error_code;

TcpServer::TcpServer(int port) {
    _serverThread = std::thread{[port, this] {
        setThreadName("tcp server acceptor");
        try {
            tcp::acceptor acceptor(TcpConnection::context(),
                                   tcp::endpoint(tcp::v4(), port));

            while (true) {
                tcp::socket socket(TcpConnection::context());
                acceptor.accept(socket);

                error_code error;

                auto connection =
                    std::make_shared<TcpConnection>(std::move(socket));
                _callback(connection);

                connection->_inThread =
                    std::thread{[conn = connection.get()] { conn->inLoop(); }};
            }
        }
        catch (std::exception &e) {
            throw e;
        }
    }};
}

void TcpServer::callback(CallbackT f) {
    _callback = f;
}
