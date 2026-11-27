#pragma once

#include "nlohmann/json.hpp"
#include <functional>
#include <mutex>

namespace lsp {

// Match responses with requests made
struct RequestQueue {
    using CallbackT = std::function<void(const nlohmann::json &)>;

    struct QueueItem {
        long id;
        CallbackT cb;
        CallbackT error;
    };

    void waitFor(long id, CallbackT callback, CallbackT error) {
        auto g = std::lock_guard{_queueMutex};
        _waitingQueue.push_back({id, callback, error});
    }

    bool operator()(const nlohmann::json &json) {
        if (auto it = json.find("id"); it != json.end()) {
            auto callback = find(it->get<long>());
            if (callback.cb) {
                callback.cb(json);
                return true;
            }
        }
        return false;
    }

    bool error(const nlohmann::json &json) {
        if (auto it = json.find("id"); it != json.end()) {
            auto callback = find(it->get<long>());
            if (callback.error) {
                callback.error(json["error"]);
                return true;
            }
        }
        return false;
    }

    void failAll() {
        auto g = std::lock_guard{_queueMutex};
        for (auto &item : _waitingQueue) {
            item.error({"id", item.id});
        }
        _waitingQueue.clear();
    }

private:
    QueueItem find(long id) {
        auto g = std::lock_guard{_queueMutex};
        for (auto it = _waitingQueue.begin(); it != _waitingQueue.end(); ++it) {
            if (it->id == id) {
                auto callback = *it;
                _waitingQueue.erase(it);
                return callback;
            }
        }

        return {};
    }

    std::mutex _queueMutex;
    std::vector<QueueItem> _waitingQueue;
};

} // namespace lsp
