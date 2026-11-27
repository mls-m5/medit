#pragma once

#include "connection.h"
#include "nlohmann/json.hpp"
#include "requestqueue.h"
#include "subscriptions.h"
#include <future>
#include <istream>
#include <stdexcept>
#include <string_view>

namespace lsp {

struct LspError : public std::runtime_error {
    nlohmann::json content;

    LspError(std::string what, nlohmann::json content)
        : std::runtime_error{what}
        , content{std::move(content)} {}
};

/// Client that makes sense of the raw data from the stream to and from the
/// lsp-server
class LspClient {
public:
    LspClient(const LspClient &) = delete;
    LspClient(LspClient &&) = delete;
    LspClient &operator=(const LspClient &) = delete;
    LspClient &operator=(LspClient &&) = delete;

    /// @param command   the command to start with, including the arguments
    LspClient(std::string command);
    ~LspClient();

    using CallbackT = std::function<void(const nlohmann::json &)>;

    /// Callback for unhandled notifications
    void callback(CallbackT f) {
        _callback = f;
    }

    void request(std::string_view method,
                 const nlohmann::json &value,
                 CallbackT callback,
                 std::function<void(const nlohmann::json)> error = 0) {
        auto id = ++_messageId;
        _handling.waitFor(id, callback, error);
        send(method, value, id);
    }

    template <typename ReqT, typename FT>
    void request(const ReqT &value,
                 FT callback,
                 std::function<void(const nlohmann::json)> error = 0) {
        CallbackT f = [callback](const nlohmann::json &json) {
            callback(json["result"]);
        };

        auto id = ++_messageId;
        _handling.waitFor(id, f, error);
        send(value, id);
    }

    /// Send notification from the client to the server
    template <typename T>
    void notify(const T &value) {
        send(T::method, value, -1);
    }

    /// Same as above but specify method
    template <typename T>
    void notify(std::string_view method, const T &value) {
        send(method, value, -1);
    }

    template <typename NotificationT>
    using SubscriptionCallbackT = std::function<void(NotificationT)>;

    template <typename T>
    void subscribe(std::function<void(const T &)> callback) {
        _subscriptions.subscribe<T>(callback);
    }

    template <typename T>
    void unsubscribe() {
        _subscriptions.unsubcribe<T>();
    }

    /// Ask the server to cleanup and prepare for a exit notification
    std::future<void> shutdown();

    /// Shut down the client without cleaning up
    void exit();

    void unexpectedShutdownCallback(Connection::ExitedFunctionT f) {
        _unexpectedExitCallback = f;
    }

private:
    // Specify negative id to send without id
    template <typename T>
    long send(const T &value, long id = -1) {
        if (id >= 0) {
            send({
                {"jsonrpc", "2.0"},
                {"id", id},
                {"method", T::method},
                {"params", value},
            });
        }
        else {
            send({
                {"jsonrpc", "2.0"},
                {"method", T::method},
                {"params", value},
            });
        }
        return id;
    }

    // Alternative way
    void send(std::string_view method,
              const nlohmann::json &json,
              long id = -1) {
        if (id >= 0) {
            send({
                {"jsonrpc", "2.0"},
                {"id", id},
                {"method", method},
                {"params", json},
            });
        }
        else {
            send({
                {"jsonrpc", "2.0"},
                {"method", method},
                {"params", json},
            });
        }
    }

    void inputThread(std::istream &in);

    void handle(const nlohmann::json &json) {
        if (auto f = json.find("error"); f != json.end()) {
            _handling.error(json);
            return;
        }
        if (!_handling(json) && !_subscriptions(json)) {
            _callback(json);
        }
    }

    void send(const nlohmann::json &json);

    RequestQueue _handling;
    Subscriptions _subscriptions;
    Connection::ExitedFunctionT _unexpectedExitCallback;
    Connection _connection;
    static inline long _messageId = 0;
    CallbackT _callback;
    bool _abort = false;
};

} // namespace lsp
