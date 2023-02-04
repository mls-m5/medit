#pragma once

#include <functional>
#include <string_view>

class IConnection {
public:
    //! Listen to received data
    virtual void subscribe(std::function<void(std::string_view)> callback) = 0;

    //! stop listening
    virtual void close() = 0;

    virtual void write(std::string_view data) = 0;
};
