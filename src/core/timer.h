#pragma once

#include <chrono>
#include <functional>
#include <mutex>
#include <vector>

using namespace std::literals;

class Timer {
    using TimeT = std::chrono::system_clock::time_point;
    using DurationT = TimeT::duration;

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
    std::vector<Trigger> _triggers;
    bool _isRunning = false;
    size_t nextRef = 0;

public:
    size_t setTimeout(DurationT duration, std::function<void()> f);

    void cancel(size_t ref);

    //! Stop the timer and end the thread (eventually)
    void stop();

    void loop();
};
