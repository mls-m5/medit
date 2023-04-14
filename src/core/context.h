#pragma once

#include "meditfwd.h"
#include <functional>

//! Keeps information of threads and timer
//! Shared for all users connected to the same server
class ThreadContext {
    IJobQueue &_jobQueue;
    IJobQueue &_guiQueue;
    ITimer &_timer;

public:
    ThreadContext(IJobQueue &jobQueue, IJobQueue &guiQueue, ITimer &timer)
        : _jobQueue(jobQueue)
        , _guiQueue(guiQueue)
        , _timer(timer) {}

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
};
