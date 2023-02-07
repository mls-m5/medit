#include "tcpconnection.h"
#include "boost/asio.hpp"
#include "core/debugoutput.h"

using namespace boost::asio;
using namespace boost::asio::ip;
using boost::system::error_code;

void TcpConnection::inLoop() {
    while (true) {
        if (!_socket.is_open()) {
            break;
        }
        if (auto l = shared_from_this()) {
            auto b = std::array<char, sizeof(size_t)>{};
            error_code ec;
            size_t len = 0;
            read(_socket, buffer(b));
            std::memcpy(&len, b.data(), sizeof(len));
            _buffer.resize(len);
            read(_socket, buffer(_buffer));

            handleInput(_buffer);
            debugOutput("got", _buffer);
        }
    }
}

void TcpConnection::handleInput(std::string_view data) {
    if (_callback) {
        _callback(data);
    }
}

void TcpConnection::subscribe(IConnection::CallbackT f) {
    _callback = f;
}

void TcpConnection::unsubscribe() {
    _callback = {};
}

void TcpConnection::close() {
    _socket.close();
}

void TcpConnection::write(std::string_view str) {
    auto b = std::array<char, sizeof(size_t)>{};
    {
        size_t len = str.length();
        std::memcpy(b.data(), &len, sizeof(len));
    }
    _socket.write_some(buffer(b));
    _socket.write_some(buffer(str));
    debugOutput("sent", str);
}

std::shared_ptr<TcpConnection> TcpConnection::connect(std::string address,
                                                      int port) {

    auto endpoints = tcp::resolver{context()}.resolve(
        tcp::resolver::query{address, std::to_string(port)});

    auto s = SocketT{context()};
    boost::asio::connect(s, endpoints);

    auto ret = std::make_shared<TcpConnection>(std::move(s));

    // Make sure to keep alive as longe as inLoop is running
    ret->_inThread = std::thread{[conn = ret.get()] { conn->inLoop(); }};

    return ret;
}

TcpConnection::~TcpConnection() {
    _socket.close();
}

void TcpConnection::waitForClose() {
    for (; _socket.is_open();) {
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
    }
}

io_context &TcpConnection::context() {
    static io_context context;
    return context;
}
