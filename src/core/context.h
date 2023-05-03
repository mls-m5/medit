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

    /// Jobs that can be done in the background without risking to mess with
    /// Any of the internal state of the program
    /// For example fetching stuff from other places, read from disk
    IJobQueue &jobQueue() {
        return _jobQueue;
    }

    /// Jobs that is done that changes the internal state of the program, such
    /// as editing buffers, changing views
    IJobQueue &guiQueue() {
        return _guiQueue;
    }

    ITimer &timer() {
        return _timer;
    }
};
