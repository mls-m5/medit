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
};
