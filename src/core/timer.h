#pragma once

#include "itimer.h"
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::literals;

class Timer : public ITimer {
public:
    //! Destroy and join thread
    ~Timer();

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
    std::vector<Trigger> _triggers;
    bool _isRunning = false;
    size_t nextRef = 0;
};
