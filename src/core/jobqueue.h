#pragma once

#include "core/threadname.h"
#include "core/threadvalidation.h"
#include "ijobqueue.h"
#include "profiler.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>

class JobQueue : public IJobQueue {

public:
    JobQueue()
        : _tv{"JobQueue"} {};
    JobQueue(JobQueue &&) = delete;
    JobQueue &operator=(JobQueue &&) = delete;
    JobQueue(const JobQueue &) = delete;
    JobQueue &operator=(const JobQueue &) = delete;

    ~JobQueue() override {
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void addTask(std::function<void()> f) override {
        if (!f) {
            throw std::runtime_error{"trying to add empty function"};
        }

        auto lock = std::unique_lock{_mutex};

        if (!_running) {
            return;
        }
        _queue.push(std::move(f));
        lock.unlock();
        _cv.notify_all();
    }

    void work(bool shouldWait = true,
              std::function<void()> afterWork = {}) override {
        {
            auto duration = ProfileDuration{};
            for (; _running;) {
                auto lock = std::unique_lock{_mutex};
                if (_queue.empty()) {
                    break;
                }
                auto task = std::move(_queue.front());
                _queue.pop();
                lock.unlock();
                task();
            }
        }

        if (afterWork) {
            afterWork();
        }

        if (shouldWait) {
            auto l = std::unique_lock(_mutex);
            _cv.wait(l, [this]() { return !_running || !_queue.empty(); });
        }
    }

    void stop() override {
        if (!_running) {
            return;
        }
        _running = false;
        {
            auto lock = std::unique_lock{_mutex};
            decltype(_queue) empty{};
            _queue.swap(empty);
        }

        _cv.notify_all();

        if (_thread.joinable()) {
            _thread.join();
        }
    }

    /// Start new thread to process incomming tasks
    void start() override {
        _thread = std::thread{[this] { loop(); }};
    }

private:
    //! Run and lock the current thread
    void loop() {
        setThreadName("jobs");
        _tv.reset();
        while (_running) {
            work();
        }
    }

    std::queue<std::function<void()>> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _running = true;
    ThreadValidation _tv;
    std::thread _thread;
};
