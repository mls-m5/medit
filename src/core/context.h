#pragma once

#include "meditfwd.h"
#include <functional>

//! Keeps information of threads and timer
class Context {
    JobQueue &_jobQueue;
    JobQueue &_guiQueue;
    Timer &_timer;
    std::function<void()> _refreshScreenFunc;

public:
    Context(JobQueue &jobQueue, JobQueue &guiQueue, Timer &timer)
        : _jobQueue(jobQueue), _guiQueue(guiQueue), _timer(timer) {}

    JobQueue &jobQueue() {
        return _jobQueue;
    }

    //! Work tok do on gui thread
    JobQueue &guiQueue() {
        return _guiQueue;
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
