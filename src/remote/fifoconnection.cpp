#include "fifoconnection.h"
#include "core/threadname.h"

FifoConnection::FifoConnection(std::filesystem::path inPath,
                               std::filesystem::path outPath) {

    _inThread = std::thread([this, inPath] {
        setThreadName("fifo in");
        _in.open(inPath);
        inLoop();
    });

    _out.open(outPath);
}

FifoConnection::~FifoConnection() {
    FifoConnection::close();
}

void FifoConnection::write(std::string_view str) {
    if (str.empty()) {
        return;
    }
    auto len = str.length();
    _out.write(reinterpret_cast<char *>(&len), sizeof(len));
    _out.write(str.data(), len);
    _out.flush();
}

void FifoConnection::subscribe(IConnection::CallbackT f) {
    _callback = f;
}

void FifoConnection::unsubscribe() {
    _callback = {};
}

void FifoConnection::close() {
    if (_in || _out) {
        _in.close();
        _out.close();
        if (_inThread.joinable()) {
            _inThread.join();
        }
    }
}

void FifoConnection::waitForClose() {
    for (; _in && _out;) {
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
    }
}

void FifoConnection::inLoop() {
    while (_in) {
        size_t len = 0;
        _in.read(reinterpret_cast<char *>(&len), sizeof(len));
        if (len == 0) {
            break;
        }
        _buffer.resize(len);
        _in.read(_buffer.data(), _buffer.size());
        handleInput(_buffer.data());
    }
}

void FifoConnection::handleInput(std::string_view data) {
    if (_callback) {
        _callback(data);
    }
}
