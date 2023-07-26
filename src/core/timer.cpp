#include "core/timer.h"
#include "core/threadname.h"
#include <algorithm>
#include <thread>

Timer::~Timer() {
    _isRunning = false;
    if (_thread.joinable()) {
        _thread.join();
    }
}

size_t Timer::setTimeout(Timer::DurationT duration, std::function<void()> f) {
    if (!_isRunning) {
        return 0;
    }
    {
        auto lock = std::scoped_lock{_mutex};

        auto time = std::chrono::system_clock::now() + duration;

        // Insert in order
        auto trigger = Trigger{time, f, ++nextRef};
        auto it = std::upper_bound(_triggers.begin(), _triggers.end(), trigger);
        _triggers.insert(it, std::move(trigger));
    }
    _cv.notify_one();
    return nextRef;
}

void Timer::cancel(size_t ref) {
    if (ref == 0) {
        return;
    }
    {
        auto lock = std::scoped_lock{_mutex};

        _triggers.erase(std::remove_if(_triggers.begin(),
                                       _triggers.end(),
                                       [ref](auto &t) { return ref == t.ref; }),
                        _triggers.end());
    }
    _cv.notify_one();
}

void Timer::stop() {
    auto lock = std::scoped_lock{_mutex};

    _triggers.clear();
    _isRunning = false;
    _cv.notify_one();
}

void Timer::loop() {
    setThreadName("timer");
    _isRunning = true;
    while (_isRunning) {
        {
            _mutex.lock();
            while (!_triggers.empty()) {
                auto nextTime = _triggers.front().time;
                if (nextTime < std::chrono::system_clock::now()) {
                    auto next = std::move(_triggers.front());
                    _triggers.erase(_triggers.begin());
                    // Make sure that it is not a deadlock if a timer
                    // callback tries to add a new callback
                    _mutex.unlock();
                    next.f();
                    _mutex.lock();
                }
                else {
                    break;
                }
            }

            _mutex.unlock();
        }

        using namespace std::literals;

        auto lock = std::unique_lock{_mutex};
        if (!_triggers.empty()) {
            _cv.wait_for(lock,
                         _triggers.front().time -
                             std::chrono::system_clock::now());
        }
        else {
            _cv.wait(lock);
        }
    }
}

void Timer::start() {
    _thread = std::thread([this] { loop(); });
}
