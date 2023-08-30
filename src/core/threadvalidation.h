#pragma once

#include <string>
#include <thread>

//! A simple class that validates that a function is called from the same thread
//! as the instance is created on
class ThreadValidation {
public:
    ThreadValidation(std::string threadName);

    void validate() const;

    void operator()() const {
        validate();
    }

    /// Validate against this thread instead
    void reset();

private:
    std::thread::id _threadId = std::this_thread::get_id();
    std::string _name;
    std::string _threadName;
};
