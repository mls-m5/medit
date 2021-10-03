#pragma once

#include "meditfwd.h"
#include <functional>

//! Keeps information of threads and timer
class Context {
    IJobQueue &_jobQueue;
    IJobQueue &_guiQueue;
    ITimer &_timer;
    std::function<void()> _refreshScreenFunc;

public:
    Context(IJobQueue &jobQueue, IJobQueue &guiQueue, ITimer &timer)
        : _jobQueue(jobQueue), _guiQueue(guiQueue), _timer(timer) {}

    IJobQueue &jobQueue() {
        return _jobQueue;
    }

    //! Work tok do on gui thread
    IJobQueue &guiQueue() {
        return _guiQueue;
    }

    ITimer &timer() {
        return _timer;
    }

    //! Trigger redraw of winow
    void redrawScreen() {
        _refreshScreenFunc();
    }

    void refreshScreenFunc(std::function<void()> f) {
        _refreshScreenFunc = f;
    }
};
