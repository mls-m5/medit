#pragma once

#include "nlohmann/json.hpp"
#include <functional>
#include <string>
#include <vector>

namespace lsp {

struct Subscriptions {
    using CallbackT = std::function<void(const nlohmann::json &)>;

    /// Handle subscriptions
    /// @return true if subscription was handled
    bool operator()(const nlohmann::json &json) {
        if (auto it = json.find("method"); it != json.end()) {
            return handle(it->get<std::string>(), json["params"]);
        }
        return false;
    }

    bool handle(const std::string &method, const nlohmann::json &json) {
        for (auto &sub : _subscriptions) {
            if (sub.first == method) {
                sub.second(json);
                return true;
            }
        }
        return false;
    }

    template <typename T>
    void subscribe(std::function<void(const T &)> callback) {
        CallbackT f = [callback](const nlohmann::json &json) {
            callback(json);
        };
        _subscriptions.push_back({std::string{T::method}, f});
    }

    template <typename T>
    void unsubcribe() {
        auto it = std::remove_if(_subscriptions.begin(),
                                 _subscriptions.end(),
                                 [](auto &s) { return s.first == T::method; });

        _subscriptions.erase(it, _subscriptions.end());
    }

    std::vector<std::pair<std::string, CallbackT>> _subscriptions;
};

} // namespace lsp
