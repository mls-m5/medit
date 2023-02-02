#include "threadvalidation.h"
#include <sstream>
#include <stdexcept>

ThreadValidation::ThreadValidation(std::string threadName)
    : _name{threadName} {}

void ThreadValidation::validate() const {
    if (std::this_thread::get_id() != _threadId) {
        auto intId = [](auto id) {
            auto ss = std::ostringstream{};
            ss << id;
            return ss.str();
        };
        throw std::runtime_error{
            "buffer called from another thread than the one started "
            "from: " +
            intId(std::this_thread::get_id()) + "(" + _name +
            ") vs started from " + intId(_threadId)};
    }
}
