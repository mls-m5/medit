#pragma once

#include "meditfwd.h"
#include <functional>

//! Keeps information of threads and timer
class Context {
    JobQueue &_jobQueue;
    Timer &_timer;
    std::function<void()> _refreshScreenFunc;

public:
    Context(JobQueue &jobQueue, Timer &timer)
        : _jobQueue(jobQueue), _timer(timer){}

    JobQueue &jobQueue() {
        return _jobQueue;
    }

    Timer &timer() {
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
