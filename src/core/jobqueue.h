#pragma once

#include <functional>
#include <mutex>
#include <queue>

class JobQueue {
    std::queue<std::function<void()>> _queue;
    std::mutex _waitMutex;
    bool _running = true;

public:
    //! Wait for another task to come in
    void wait() {
        _waitMutex.lock();
        _waitMutex.unlock();
    }

    void addTask(std::function<void()> f) {
        if (_queue.empty()) {
            _queue.push(std::move(f));
            _waitMutex.try_lock();
            _waitMutex.unlock();
        }
        else {
            _queue.push(std::move(f));
        }
    }

    void work() {
        _waitMutex.lock();
        while (!_queue.empty() && _running) {
            auto task = std::move(_queue.front());
            _queue.pop();
            task();
        }

        wait();
    }

    void stop() {
        _running = false;
        _waitMutex.try_lock();
        decltype(_queue) empty{};
        _queue.swap(empty);
        _waitMutex.unlock();
    }

    void loop() {
        while (_running) {
            work();
        }
    }
};
