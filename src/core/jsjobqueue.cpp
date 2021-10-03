#ifdef __EMSCRIPTEN__
#include "jsjobqueue.h"
#include <emscripten.h>
#include <emscripten/html5.h>

namespace {

EMSCRIPTEN_KEEPALIVE
void work(void *data) {
    auto queue = static_cast<JsJobQueue *>(data);

    queue->work(false);
}

} // namespace

void JsJobQueue::addTask(std::function<void()> task) {
    _tasks.push_back(task);
    if (!_isWaiting) {
        _isWaiting = true;

        emscripten_set_timeout(::work, 1000, this);
        EM_ASM(console.log("add task timeout"));
    }
}

void JsJobQueue::work(bool shouldWait) {
    while (!_tasks.empty()) {
        _tasks.front()();
        _tasks.pop_front();
    }
    _isWaiting = false;
}

void JsJobQueue::start() {
    _isRunning = false;
}

void JsJobQueue::stop() {
    // Just wait
}

#endif // __EMSCRIPTEN__
