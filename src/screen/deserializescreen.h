#pragma once

#include "iconnection.h"
#include "iscreen.h"
#include "nlohmann/json.hpp"
#include <memory>

class DeserializeScreen : public IConnection {
public:
    DeserializeScreen(std::shared_ptr<IScreen> screen);

    DeserializeScreen() = delete;
    DeserializeScreen(const DeserializeScreen &) = delete;
    DeserializeScreen(DeserializeScreen &&) = delete;
    DeserializeScreen &operator=(const DeserializeScreen &) = delete;
    DeserializeScreen &operator=(DeserializeScreen &&) = delete;

    // IConnection interface
    void subscribe(std::function<void(std::string_view)> callback) override;
    void close() override;
    void write(std::string_view data) override;

private:
    void handle(const nlohmann::json &json);

    //! Outgoing data from this class
    void send(const nlohmann::json &data);

    std::shared_ptr<IScreen> _screen;
    std::function<void(std::string_view)> _callback;
};
