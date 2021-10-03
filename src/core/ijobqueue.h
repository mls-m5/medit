#pragma once
#include <functional>

//! Job queues handles doing jobs "sometime else"
//! Notice that for javascript this might be done on the same thread
class IJobQueue {
public:
    virtual ~IJobQueue() = default;

    virtual void addTask(std::function<void()>) = 0;
    virtual void work(bool shouldWait = true) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};
