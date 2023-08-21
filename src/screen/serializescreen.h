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
    ~SerializeScreen() override;

    SerializeScreen() = delete;
    SerializeScreen(const SerializeScreen &) = delete;
    SerializeScreen(SerializeScreen &&) = delete;
    SerializeScreen &operator=(const SerializeScreen &) = delete;
    SerializeScreen &operator=(SerializeScreen &&) = delete;

    // IScreen interface
    void draw(size_t x, size_t y, FStringView str) override;
    void refresh() override;
    void clear() override;
    void cursor(size_t x, size_t y) override;
    void title(std::string title) override;
    void palette(const Palette &palette) override;
    size_t addStyle(const Color &foreground,
                    const Color &background,
                    size_t index) override;
    void cursorStyle(CursorStyle) override;

    /// see @IInput
    void subscribe(CallbackT f) override;
    /// see @IInput
    void unsubscribe() override;

    std::string clipboardData() override;

    //! Set clipboard data on clients computer
    void clipboardData(std::string) override;

private:
    std::shared_ptr<IConnection> _connection;
    volatile std::atomic_long _currentRequest = 1;
    volatile std::atomic_long _receivedRequest = 0;
    nlohmann::json createRequest(std::string name);

    void send(const nlohmann::json &);
    nlohmann::json request(std::string_view method);

    //! Where the remote data enters this class
    //    void receive(const nlohmann::json &);
    //    void receive(Archive &);
    void receive(std::string_view data);

    std::mutex _mutex;
    std::condition_variable _cv;

    //    nlohmann::json _receivedData;
    std::string _receivedData;

    CallbackT _callback;
};
