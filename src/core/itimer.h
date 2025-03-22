#pragma once

#include <chrono>
#include <functional>

class ITimer {
public:
    using TimeT = std::chrono::system_clock::time_point;
    using DurationT = TimeT::duration;

    ITimer() = default;
    ITimer(const ITimer &) = delete;
    ITimer(ITimer &&) = delete;
    ITimer &operator=(const ITimer &) = delete;
    ITimer &operator=(ITimer &&) = delete;
    virtual ~ITimer() = default;

    virtual size_t setTimeout(DurationT duration, std::function<void()> f) = 0;

    virtual void cancel(size_t ref) = 0;

    //! Stop the timer and end the threads execution (eventually)
    virtual void stop() = 0;

    virtual void loop() = 0;

    virtual void start() = 0;
};
