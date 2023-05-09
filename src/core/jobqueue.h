#pragma once

#include "ijobqueue.h"
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class JobQueue : public IJobQueue {

public:
    JobQueue() = default;
    JobQueue(const JobQueue &) = delete;
    JobQueue &operator=(const JobQueue &) = delete;

    ~JobQueue() {
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void addTask(std::function<void()> f) override {
        _queue.push(std::move(f));
        _waitMutex.try_lock();
        _waitMutex.unlock();
    }

    void work(bool shouldWait = true) override {
        if (shouldWait) {
            _waitMutex.lock();
        }
        while (!_queue.empty() && _running) {
            auto task = std::move(_queue.front());
            _queue.pop();
            task();
        }

        if (shouldWait) {
            wait();
        }
    }

    void stop() override {
        if (!_running) {
            return;
        }
        _running = false;
        _waitMutex.try_lock();
        decltype(_queue) empty{};
        _queue.swap(empty);
        _waitMutex.unlock();
    }

    void start() override {
        _thread = std::thread{[this] { loop(); }};
    }

private:
    //! Run and lock the current thread
    void loop() {
        _threadId = std::this_thread::get_id();
        while (_running) {
            work();
        }
    }

    //! Wait for another task to come in
    void wait() {
        auto l = std::scoped_lock(_waitMutex);
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

    std::queue<std::function<void()>> _queue;
    std::mutex _waitMutex;
    bool _running = true;
    std::thread::id _threadId = {};
    std::thread _thread;
};

