#pragma once

#include <functional>
#include <string_view>

class IConnection {
public:
    using CallbackT = std::function<void(std::string_view)>;

    //! Listen to received data
    virtual void subscribe(CallbackT callback) = 0;

    virtual void unsubscribe() = 0;

    //! stop listening
    virtual void close() = 0;

    virtual void write(std::string_view data) = 0;

    /// For the object that owns the connection, if the object does not own the
    ///  connection it should just return immediately
    virtual void waitForClose() = 0;
};

inline void connect(IConnection &a, IConnection &b) {
    a.subscribe([&b](auto data) { b.write(data); });
    b.subscribe([&a](auto data) { a.write(data); });
}
