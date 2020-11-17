#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class Timer {
    struct Trigger {
        std::chrono::time_point<double> time;
        std::function<void()> f;
        size_t ref;
    };

    std::mutex _waitMutex;
    std::mutex _mutex;
    std::vector<Trigger> _triggers;
    bool _isRunning = false;
    size_t nextRef = 0;

public:
    void setTimeout(std::chrono::duration<double> duration,
                    std::function<void()> f) {

        if (!_isRunning) {
            return;
        }
        auto lock = std::scoped_lock{_mutex};

        for (auto it = _triggers.begin(); it != _triggers.end(); ++it) {
        }

        auto time = std::chrono::high_resolution_clock::now() + duration;

        _triggers.push_back(Trigger{time, f, ++nextRef});
        //! Todo insert in order
    }

    void cancel(size_t ref) {
        auto lock = std::scoped_lock{_mutex};

        _triggers.erase(std::remove_if(_triggers.begin(),
                                       _triggers.end(),
                                       [ref](auto &t) { return ref == t.ref; }),
                        _triggers.end());
    }

    void loop() {
        _isRunning = true;
        while (_isRunning) {
            {
                _mutex.lock();
                while (!_triggers.empty()) {
                    auto next = _triggers.front();
                    _triggers.erase(_triggers.begin());
                    _mutex.unlock();

                    auto time = std::chrono::high_resolution_clock::now();
                    if (next.time)
                        next.f();

                    _mutex.lock();
                }

                //! Exiting with locked mutex
                bool shouldWait = false;
                if (!_triggers.empty()) {
                    auto textTime = _triggers.front().time;
                }
                else {
                    shouldWait = true;
                }
                _mutex.unlock();
            }

            using namespace std::literals;

            std::this_thread::sleep_for(.1s);
        }
    }
};
