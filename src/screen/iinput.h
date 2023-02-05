#pragma once

#include "keys/event.h"
#include <functional>

class IInput {
public:
    using EventListT = std::vector<Event>;
    using CallbackT = std::function<void(EventListT)>;

    virtual void subscribe(CallbackT f) = 0;

    virtual void unsubscribe() = 0;

    virtual ~IInput() = default;
};
