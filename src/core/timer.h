#pragma once

#include "itimer.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::literals;

class Timer : public ITimer {
public:
    Timer() = default;
    Timer(const Timer &) = delete;
    Timer(Timer &&) = delete;
    Timer &operator=(const Timer &) = delete;
    Timer &operator=(Timer &&) = delete;

    //! Destroy and join thread
    ~Timer() override;

    size_t setTimeout(DurationT duration, std::function<void()> f) override;

    void cancel(size_t ref) override;

    void stop() override;

    void loop() override;

    void start() override;

private:
    struct Trigger {
        TimeT time;
        std::function<void()> f;
        size_t ref;

        bool operator<(const Trigger &other) const {
            return time < other.time;
        }
    };

    std::mutex _waitMutex;
    std::mutex _mutex;
    std::thread _thread;
    std::condition_variable _cv;
    std::vector<Trigger> _triggers;
    bool _isRunning = false;
    size_t nextRef = 0;
};
