#pragma once

#include <thread>

//! A simple class that validates that a function is called from the same thread
//! as the instance is created on
class ThreadValidation {
public:
    void validate() const;

    void operator()() const {
        validate();
    }

private:
    const std::thread::id _threadId = std::this_thread::get_id();
};
