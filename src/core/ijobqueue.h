#pragma once
#include <functional>

//! Job queues handles doing jobs "sometime else"
//! Notice that for javascript this might be done on the same thread
class IJobQueue {
public:
    IJobQueue() = default;
    IJobQueue(const IJobQueue &) = delete;
    IJobQueue(IJobQueue &&) = delete;
    IJobQueue &operator=(const IJobQueue &) = delete;
    IJobQueue &operator=(IJobQueue &&) = delete;
    virtual ~IJobQueue() = default;

    virtual void addTask(std::function<void()>) = 0;
    virtual void work(bool shouldWait = true,
                      std::function<void()> afterWork = {}) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};
