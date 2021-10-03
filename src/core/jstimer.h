#pragma once

#include "core/itimer.h"

#ifdef __EMSCRIPTEN__

class JsTimer : public ITimer {
public:
    size_t setTimeout(DurationT duration, std::function<void()> f) override;
    void cancel(size_t ref) override;
    void stop() override;
    void loop() override;
    void start() override;
};

#endif // __EMSCRIPTEN__
