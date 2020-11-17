#pragma once

#include "meditfwd.h"

//! Keeps information of threads and timer
class Context {
    JobQueue &_jobQueue;
    Timer &_timer;

public:
    Context(JobQueue &jobQueue, Timer &timer)
        : _jobQueue(jobQueue), _timer(timer) {}

    JobQueue &jobQueue() {
        return _jobQueue;
    }

    Timer &timer() {
        return _timer;
    }
};
