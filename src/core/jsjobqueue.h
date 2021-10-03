#pragma once

#ifdef __EMSCRIPTEN__

#include "ijobqueue.h"
#include <deque>
#include <functional>

//! A job que that runs on top on javascript (single threaded)
//! That means that in practice the javascript version is single treaded
class JsJobQueue : public IJobQueue {
public:
    void addTask(std::function<void()>);
    void work(bool shouldWait);
    void start();
    void stop();

private:
    bool _isRunning = false;
    bool _isWaiting = false; // If the jobqueue has asked js to execute
    std::deque<std::function<void()>> _tasks;
};

#endif //__EMSCRIPTEN
