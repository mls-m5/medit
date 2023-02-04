#pragma once

#include "iconnection.h"
#include "nlohmann/json.hpp"
#include "screen/iscreen.h"
#include <atomic>
#include <condition_variable>
#include <mutex>

//! Screen used on server to serialize screen calls to binary form
class SerializeScreen : public IScreen {
public:
    SerializeScreen(std::shared_ptr<IConnection> connection);
    ~SerializeScreen();

    SerializeScreen() = delete;
    SerializeScreen(const SerializeScreen &) = delete;
    SerializeScreen(SerializeScreen &&) = delete;
    SerializeScreen &operator=(const SerializeScreen &) = delete;
    SerializeScreen &operator=(SerializeScreen &&) = delete;

    // IScreen interface
    void draw(size_t x, size_t y, const FString &str);
    void refresh();
    void clear();
    void cursor(size_t x, size_t y);
    size_t x() const;
    size_t y() const;
    size_t width() const;
    size_t height() const;
    void title(std::string title);
    const IPalette &palette() const;
    IPalette &palette();
    size_t addStyle(const Color &foreground,
                    const Color &background,
                    size_t index);
    void cursorStyle(CursorStyle);

private:
    std::shared_ptr<IConnection> _connection;
    volatile std::atomic_long _currentRequest = 1;
    volatile std::atomic_long _receivedRequest = 0;
    nlohmann::json createRequest(std::string name);

    void send(const nlohmann::json &);
    nlohmann::json request(nlohmann::json);

    //! Where the remote data enters this class
    void receive(const nlohmann::json &);

    std::mutex _mutex;
    std::condition_variable _cv;

    nlohmann::json _receivedData;
};
