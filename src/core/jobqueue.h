#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class JobQueue {
    std::queue<std::function<void()>> _queue;
    std::mutex _waitMutex;
    bool _running = true;
    std::thread::id _threadId = {};

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
        _threadId = std::this_thread::get_id();
        while (_running) {
            work();
        }
    }

    void forceThisThread() {
        if (!isThisThread()) {
            throw std::runtime_error("function called from wrong thread");
        }
    }

    //! @returns true if the calling thread is the same as the one called loop
    bool isThisThread() {
        return std::this_thread::get_id() == _threadId;
    }
};
