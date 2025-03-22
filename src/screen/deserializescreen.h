#pragma once

#include "iconnection.h"
#include "iscreen.h"
#include <memory>
#include <string_view>
#include <vector>

class DeserializeScreen : public IConnection {
public:
    DeserializeScreen(std::shared_ptr<IScreen> screen);

    DeserializeScreen() = delete;
    DeserializeScreen(const DeserializeScreen &) = delete;
    DeserializeScreen(DeserializeScreen &&) = delete;
    DeserializeScreen &operator=(const DeserializeScreen &) = delete;
    DeserializeScreen &operator=(DeserializeScreen &&) = delete;
    ~DeserializeScreen() = default;

    // IConnection interface
    void subscribe(std::function<void(std::string_view)> callback) override;
    void close() override;
    void write(std::string_view data) override;
    void unsubscribe() override;
    void waitForClose() override {};

private:
    void handle(std::string_view);

    //! Outgoing data from this class
    void send(std::string_view);

    void screenCallback(IScreen::EventListT list);

    std::shared_ptr<IScreen> _screen;
    std::function<void(std::string_view)> _callback;
    std::vector<std::string> _unhandledQueue;
};
